#include "BootInfo.h"
#include "BasicRenderer.h"
#include "IDT.h"
#include "GDT.h"
#include "Keyboard.h"
#include "IO.h"
#include "Heap.h"
#include "PIT.h"
#include "str.h"
#include "fat32.h"
#include "Vector.h"
#include "AHCI.h"
#include "MBR.h"
#include "Syscalls.h"
#include "Memory.h"

extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

extern "C" void JumpToUser(uint_64 rip, uint_64 rsp);

extern "C" int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) { return 0; }
void* __dso_handle = (void*)0;

void* operator new(unsigned long size) { 
    void* ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr; 
}
void* operator new[](unsigned long size) { 
    void* ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr; 
}
void operator delete(void* p) { free(p); }
void operator delete(void* p, unsigned long size) { free(p); }
void operator delete[](void* p) { free(p); }
void operator delete[](void* p, unsigned long size) { free(p); }

static Framebuffer kernel_fb;
static PSF1_Font kernel_font;

String operator+(const char* lhs, const String& rhs) {
    String res = lhs; 
    return res + rhs; 
}

// --- Storage Infrastructure ---

bool DiskReadWrapper(uint64_t lba, uint32_t count, void* buffer) {
    if (AHCI::GlobalAHCIDriver != nullptr && AHCI::GlobalAHCIDriver->portCount > 0) {
        if (AHCI::GlobalAHCIDriver->ports[0] != nullptr) {
            return AHCI::GlobalAHCIDriver->ports[0]->Read(lba, count, buffer);
        }
    }
    return false;
}

FAT32::Driver* globalFat32Driver = nullptr;

FAT32::Driver* InitStorage() {
    if (!AHCI::GlobalAHCIDriver || AHCI::GlobalAHCIDriver->portCount == 0) return nullptr;

    uint8_t* sector0 = (uint8_t*)malloc(512);
    if (!DiskReadWrapper(0, 1, sector0)) {
        if (GlobalRenderer) GlobalRenderer->Print("Disk Read Error (LBA 0)\n", 0xFFFF0000);
        free(sector0);
        return nullptr;
    }

    // Check for 0xAA55 signature
    if (sector0[510] != 0x55 || sector0[511] != 0xAA) {
        if (GlobalRenderer) GlobalRenderer->Print("No Boot Signature (0xAA55 missing)\n", 0xFFFF0000);
        free(sector0);
        return nullptr;
    }

    // 1. Try to treat it as MBR
    MasterBootRecord* mbr = (MasterBootRecord*)sector0;
    uint32_t partitionOffset = 0;
    bool foundFat32 = false;
    
    for (int i = 0; i < 4; i++) {
        if (mbr->primaryPartition[i].partition_id == 0x0B || mbr->primaryPartition[i].partition_id == 0x0C) {
            partitionOffset = mbr->primaryPartition[i].lba_start;
            foundFat32 = true;
            break;
        }
    }

    if (foundFat32) {
        if (GlobalRenderer) GlobalRenderer->Print("Found MBR partition.\n");
        FAT32::Driver* driver = new FAT32::Driver(DiskReadWrapper, partitionOffset);
        free(sector0);
        return driver;
    }

    // 2. Try to treat it as a FAT32 Boot Sector (no partition table)
    // Check for "FAT" string in the EBR (offset 0x52 or 0x36)
    bool sig52 = (sector0[0x52] == 'F' && sector0[0x53] == 'A' && sector0[0x54] == 'T');
    bool sig36 = (sector0[0x36] == 'F' && sector0[0x37] == 'A' && sector0[0x38] == 'T');

    if (sig52 || sig36) {
        if (GlobalRenderer) GlobalRenderer->Print("Found FAT Boot Sector.\n");
        FAT32::Driver* driver = new FAT32::Driver(DiskReadWrapper, 0);
        free(sector0);
        return driver;
    }

    if (GlobalRenderer) {
        GlobalRenderer->Print("Unknown Partition. Sig52: ");
        char dump[4] = {(char)sector0[0x52], (char)sector0[0x53], (char)sector0[0x54], 0};
        GlobalRenderer->Print(dump);
        GlobalRenderer->Print(" Sig36: ");
        char dump2[4] = {(char)sector0[0x36], (char)sector0[0x37], (char)sector0[0x38], 0};
        GlobalRenderer->Print(dump2);
        GlobalRenderer->Print("\n", 0xFFFF0000);
    }
    free(sector0);
    return nullptr;
}

char* simple_split(char* str, char delim) {
    if (str == 0) return 0;

    while (*str != '\0') {
        if (*str == delim) {
            *str = '\0';      // Cut the string here
            return str + 1;   // Return the start of the next part
        }
        str++;
    }
    return 0; // No more delimiters found
}


// --- Kernel Main ---

void Init(BootInfo* bootInfo){
    if (bootInfo == nullptr || bootInfo->fb == nullptr) {
        while(1) __asm__("hlt");
    }

    // 1. Core Hardware & Memory initialization
    InitializeGDT();
    InitializeHeap(0x4000000, 0x1000000); // Move heap to 64MB - safer for most systems

    kernel_fb = *(bootInfo->fb);
    if (bootInfo->font && bootInfo->font->header) {
        kernel_font = *(bootInfo->font);
    } else {
        // Fallback to embedded font if bootloader didn't provide one
        PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
        kernel_font.header = fontHeader;
        kernel_font.glyphBuffer = (void*)((uint_64)fontHeader + sizeof(PSF1_Header));
    }

    GlobalRenderer = new BasicRenderer(&kernel_fb, &kernel_font);
    GlobalRenderer->Clear(0x00111111); 
    GlobalRenderer->Print("\nSYSTEM 15 Kernel Booting...\n", 0xFF00FF00);

    // 2. Extended Hardware Initialization
    GlobalRenderer->Print("Initializing IDT... ");
    InitializeIDT();
    GlobalRenderer->Print("Done.\n", 0xFF00FF00);

    uint_64 kernel_stack;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(kernel_stack));
    GlobalTSS.rsp0 = kernel_stack;

    MainKeyboardHandler = KeyboardHandler;
    PIT::SetFrequency(100);

    GlobalRenderer->Print("Initializing AHCI... ");
    // AHCI::Init();
    GlobalRenderer->Print("Done.\n", 0xFF00FF00);

    activate_sse();

    GlobalRenderer->Print("Enabling Interrupts... ");
    __asm__ volatile ("sti");
    GlobalRenderer->Print("Done.\n", 0xFF00FF00);

    // 2. Storage Initialization
    GlobalRenderer->Print("Initializing FS");
    globalFat32Driver = InitStorage();
    if (globalFat32Driver) {
        GlobalRenderer->Print("FAT32 Mounted.\n", 0xFF00FF00);
    } else {
        GlobalRenderer->Print("No FAT32 found.\n", 0xFFFFFF00);
    }

    GlobalRenderer->Print("SYSCALLS DISABLED... \n");
    // InitializeSyscalls();
    // GlobalRenderer->Print("Done.\n", 0xFF00FF00);

    GlobalRenderer->Print("Ready.\n");

    // } else {
    //     // Fallback for emulators/raw images without MBR
    //     if (AHCI::GlobalAHCIDriver && AHCI::GlobalAHCIDriver->portCount > 0) {
    //         globalFat32Driver = new FAT32::Driver(DiskReadWrapper, 0);
    //         GlobalRenderer->Print("FAT32 Mounted (Superfloppy / Raw).\n", 0xFFFFFF00);
    //     }
    // }
}

String path = "/";

int split_string(char* input, char delimiter, char** tokens, int max_tokens) {
    if (!input || max_tokens <= 0) return 0;

    int count = 0;
    char* ptr = input;
    
    // The first token starts at the beginning of the string
    tokens[count++] = ptr;

    while (*ptr != '\0') {
        if (*ptr == delimiter) {
            // Null-terminate the current token
            *ptr = '\0';
            
            // The next token starts after this delimiter
            if (count < max_tokens) {
                tokens[count++] = ptr + 1;
            } else {
                break; // Reached max token limit
            }
        }
        ptr++;
    }

    return count;
}

void InputMan(String input){
        if (input.size() == 0) return;
        
        if (input == "clear") {
            GlobalRenderer->Clear(GlobalRenderer->ClearColor);
        } 
        else if (input == "time") {
            GlobalRenderer->Print("Time since boot: ");
            GlobalRenderer->Print(IntegerToString(PIT::TimeSinceBootMS / 1000));
            GlobalRenderer->Print(".");
            uint_64 ms_frac = (PIT::TimeSinceBootMS % 1000) / 10;
            if (ms_frac < 10) GlobalRenderer->Print("0");
            GlobalRenderer->Print(IntegerToString(ms_frac));
            GlobalRenderer->Print(" s\n");
        } 
        else if (input == "fs") {
            if (AHCI::GlobalAHCIDriver == nullptr) {
                GlobalRenderer->Print("FS Error: AHCI Pointer is NULL\n", 0xFFFF0000);
            } else {
                GlobalRenderer->Print("AHCI Status: LIVE\n");
                GlobalRenderer->Print("Pointer: ");
                GlobalRenderer->Print(HexToString((uint64_t)AHCI::GlobalAHCIDriver));
                GlobalRenderer->Print("\nActive Ports: ");
                GlobalRenderer->Print(IntegerToString(AHCI::GlobalAHCIDriver->portCount));
                GlobalRenderer->Print("\n");
                
                if (globalFat32Driver) {
                    GlobalRenderer->Print("FAT32 is currently Mounted.\n", 0xFF00FF00);
                } else {
                    GlobalRenderer->Print("FAT32 is NOT Mounted.\n", 0xFFFF0000);
                }
            }
        } 
        else if (input == "ls") {
            if (!globalFat32Driver) {
                GlobalRenderer->Print("Error: File system not mounted.\n");
            } else {
                Vector<FAT32::File> files = globalFat32Driver->ListRootDirectory();
                if (files.size() == 0) {
                    GlobalRenderer->Print("Directory is empty.\n");
                } else {
                    for (int i = 0; i < files.size(); i++) {
                        GlobalRenderer->Print(files[i].Name);
                        if (files[i].IsDirectory) {
                            GlobalRenderer->Print(" [DIR]\n");
                        } else {
                            GlobalRenderer->Print(" (");
                            GlobalRenderer->Print(IntegerToString(files[i].Size));
                            GlobalRenderer->Print(" bytes)\n");
                        }
                    }
                }
            }
        } 
        else if (input == "read") {
            if (!globalFat32Driver) {
                GlobalRenderer->Print("Error: File system not mounted.\n");
                return;
            }
            
            String filename = prompt("Enter filename: ");
            Vector<FAT32::File> files = globalFat32Driver->ListRootDirectory();
            bool found = false;
            
            for (int i = 0; i < files.size(); i++) {
                if (files[i].Name == filename && !files[i].IsDirectory) {
                    found = true;
                    uint8_t* fileData = globalFat32Driver->ReadFile(files[i]);
                    
                    if (fileData) {
                        GlobalRenderer->Print("\n--- ");
                        GlobalRenderer->Print(filename);
                        GlobalRenderer->Print(" ---\n");
                        
                        // Print file. Note: This assumes it is a null-terminated string/text file.
                        // If it is binary, printing it directly might cause display glitches.
                        GlobalRenderer->Print((const char*)fileData);
                        GlobalRenderer->Print("\n--- END ---\n");
                        
                        free(fileData);
                    } else {
                        GlobalRenderer->Print("Error reading file into memory.\n", 0xFFFF0000);
                    }
                    break;
                }
            }
            
            if (!found) {
                GlobalRenderer->Print("File not found.\n", 0xFFFF0000);
            }
        }
        else if (input == "run") {
            if (!globalFat32Driver) {
                GlobalRenderer->Print("Error: File system not mounted.\n");
                return;
            }
            
            String filename = prompt("Enter executable: ");
            Vector<FAT32::File> files = globalFat32Driver->ListRootDirectory();
            bool found = false;
            
            for (int i = 0; i < files.size(); i++) {
                if (files[i].Name == filename && !files[i].IsDirectory) {
                    found = true;
                    uint8_t* fileData = globalFat32Driver->ReadFile(files[i]);
                    
                    if (fileData) {
                        GlobalRenderer->Print("Loading ");
                        GlobalRenderer->Print(filename);
                        GlobalRenderer->Print("...\n");

                        // Allocate a user stack
                        void* userStack = malloc(0x4000);
                        uint_64 userRsp = (uint_64)userStack + 0x4000;

                        GlobalRenderer->Print("Jumping to Ring 3...\n");
                        JumpToUser((uint_64)fileData, userRsp);
                    } else {
                        GlobalRenderer->Print("Error reading file.\n");
                    }
                    break;
                }
            }
            if (!found) GlobalRenderer->Print("File not found.\n");
        }
        else if (input == "help") {
            GlobalRenderer->Print("Available commands: clear, time, fs, ls, run, read, help\n");
        } else if (input == "cd") {
            String x = prompt("DIR > ");
            path = x;
        }
        else {
            GlobalRenderer->Print("Unknown command: ");
            GlobalRenderer->Print(input);
            GlobalRenderer->Print("\n");
        }
    }

extern "C" void kernel_main(BootInfo* bootInfo) {
    Init(bootInfo);
    while(1) { 
        GlobalRenderer->Print(">");
        String input = prompt(String("adam@OS:") + path + "> ");
        InputMan(input);
    }

}