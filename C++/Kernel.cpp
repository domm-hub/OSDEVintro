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
#include "PageFrameAllocator.h"
#include "Paging.h"

extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

extern "C" void JumpToUser(uint_64 rip, uint_64 rsp);
extern "C" uint_64 KernelStackPtr;

extern "C" int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) { return 0; }
void* __dso_handle = (void*)0;

uint_64 RunningProcesses = 0;

// --- Storage Globals ---
FAT32::Driver* globalFat32Driver = nullptr;
uint32_t currentDirCluster = 2; 
String* currentPath = nullptr; 

// External function from str.cpp
Vector<String> split(String text, char delimiter);

String ReadDiskHelper(FAT32::File file) {
    if (!globalFat32Driver) return String("");
    
    uint8_t* volatile buffer = globalFat32Driver->ReadFile(file);
    if (buffer == nullptr) return String("");

    String result((const char*)buffer); 
    free((void*)buffer); 

    return result;
}

// --- C++ Memory Operators ---
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

// --- Global Paging State ---
Paging::PageTableManager* GlobalPageTableManager = nullptr;

// --- Storage Infrastructure ---

bool DiskReadWrapper(uint64_t lba, uint32_t count, void* buffer) {
    if (AHCI::GlobalAHCIDriver != nullptr && AHCI::GlobalAHCIDriver->portCount > 0) {
        if (AHCI::GlobalAHCIDriver->ports[0] != nullptr) {
            return AHCI::GlobalAHCIDriver->ports[0]->Read(lba, count, buffer);
        }
    }
    return false;
}

FAT32::Driver* InitStorage() {
    if (!AHCI::GlobalAHCIDriver || AHCI::GlobalAHCIDriver->portCount == 0) return nullptr;

    uint8_t* sector0 = (uint8_t*)malloc(512);
    if (!DiskReadWrapper(0, 1, sector0)) {
        free(sector0);
        return nullptr;
    }

    if (sector0[510] != 0x55 || sector0[511] != 0xAA) {
        free(sector0);
        return nullptr;
    }

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
        FAT32::Driver* driver = new FAT32::Driver(DiskReadWrapper, partitionOffset);
        free(sector0);
        return driver;
    }

    bool sig52 = (sector0[0x52] == 'F' && sector0[0x53] == 'A' && sector0[0x54] == 'T');
    bool sig36 = (sector0[0x36] == 'F' && sector0[0x37] == 'A' && sector0[0x38] == 'T');

    if (sig52 || sig36) {
        FAT32::Driver* driver = new FAT32::Driver(DiskReadWrapper, 0);
        free(sector0);
        return driver;
    }

    free(sector0);
    return nullptr;
}

void Init(BootInfo* bootInfo){
    if (bootInfo == nullptr || bootInfo->fb == nullptr) {
        while(1) __asm__("hlt");
    }

    InitializeGDT();
    GlobalAllocator.ReadMemoryMap(bootInfo->memoryMap, bootInfo->memoryMapSize, bootInfo->descriptorSize);

    Paging::PageTable* pml4 = (Paging::PageTable*)GlobalAllocator.RequestPage();
    memset(pml4, 0, 4096);
    GlobalPageTableManager = new Paging::PageTableManager(pml4);

    for (uint_64 i = 0; i < 0x100000000; i += 4096) {
        GlobalPageTableManager->MapMemory((void*)i, (void*)i, false);
    }

    uint_64 fbSize = (uint_64)bootInfo->fb->BufferSize;
    uint_64 fbBase = (uint_64)bootInfo->fb->BaseAddress;
    GlobalAllocator.ReservePages((void*)fbBase, fbSize / 4096 + 1);
    for (uint_64 i = fbBase; i < fbBase + fbSize; i += 4096) {
        GlobalPageTableManager->MapMemory((void*)i, (void*)i, false);
    }

    LoadCR3((uint_64)pml4);

    InitializeHeap(0x4000000, 0x1000000); 

    kernel_fb = *(bootInfo->fb);
    if (bootInfo->font && bootInfo->font->header) {
        kernel_font = *(bootInfo->font);
    } else {
        PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
        kernel_font.header = fontHeader;
        kernel_font.glyphBuffer = (void*)((uint_64)fontHeader + sizeof(PSF1_Header));
    }

    GlobalRenderer = new BasicRenderer(&kernel_fb, &kernel_font);
    GlobalRenderer->Clear(0x00111111); 
    GlobalRenderer->Print("\nSYSTEM 15 Kernel Booting...\n", 0xFF00FF00);

    InitializeIDT();

    uint_64 kernel_stack;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(kernel_stack));
    GlobalTSS.rsp0 = kernel_stack;

    MainKeyboardHandler = KeyboardHandler;
    PIT::SetFrequency(100);

    __asm__ volatile ("mov %%rsp, %0" : "=m"(KernelStackPtr));

    InitializeSyscalls();
    AHCI::Init();

    __asm__ volatile ("sti");

    globalFat32Driver = InitStorage();
    GlobalRenderer->Print("Ready.\n");
}

void InputMan(String input){
    input.Trim();
    if (input.size() == 0) return;
    
    Vector<String> parts = split(input, ' ');
    String cmd = parts[0];
    cmd.ToUpper();

    if (cmd == "CLEAR") {
        GlobalRenderer->Clear(GlobalRenderer->ClearColor);
    } 
    else if (cmd == "LS") {
        if (!globalFat32Driver) {
            GlobalRenderer->Print("FS not mounted.\n");
            return;
        }
        
        uint32_t targetCluster = currentDirCluster;
        if (parts.size() > 1) {
            String dirname = parts[1];
            dirname.ToUpper();
            
            if (dirname == ".") targetCluster = currentDirCluster;
            else {
                Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
                bool found = false;
                for (int i = 0; i < files.size(); i++) {
                    String entryName = files[i].Name;
                    entryName.ToUpper();
                    if (entryName == dirname && files[i].IsDirectory) {
                        targetCluster = files[i].FirstCluster;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    GlobalRenderer->Print("Directory not found: ");
                    GlobalRenderer->Print(dirname.c_str());
                    GlobalRenderer->Print("\n");
                    return;
                }
            }
        }

        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(targetCluster);
        if (files.size() == 0) {
            GlobalRenderer->Print("[Empty Directory]\n");
        } else {
            for (int i = 0; i < files.size(); i++) {
                GlobalRenderer->Print(files[i].Name.c_str());
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
    else if (cmd == "CD") {
        if (!globalFat32Driver) return;
        
        String dirname;
        if (parts.size() > 1) {
            dirname = parts[1];
        } else {
            dirname = prompt("Dir > ");
        }
        dirname.Trim();
        dirname.ToUpper();

        if (dirname == "/") {
            currentDirCluster = 2;
            *currentPath = "/";
            return;
        }

        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();
            if (entryName == dirname && files[i].IsDirectory) {
                currentDirCluster = files[i].FirstCluster;
                
                if (dirname == "..") {
                    // Update path string (find last / and cut)
                    if (*currentPath != "/") {
                        int lastSlash = -1;
                        for (int j = 0; j < (int)currentPath->length(); j++) {
                            if ((*currentPath)[j] == '/') lastSlash = j;
                        }
                        if (lastSlash == 0) *currentPath = "/";
                        else {
                            // Cut string at lastSlash
                            String newPath = "";
                            for (int j = 0; j < lastSlash; j++) newPath.add((*currentPath)[j]);
                            *currentPath = newPath;
                        }
                    }
                } else if (dirname != ".") {
                    if (*currentPath == "/") *currentPath = "/" + files[i].Name;
                    else *currentPath = *currentPath + "/" + files[i].Name;
                }
                return;
            }
        }
        GlobalRenderer->Print("Directory not found.\n");
    }
    else if (cmd == "RUN") {
        if (!globalFat32Driver) return;
        
        String filename;
        if (parts.size() > 1) {
            filename = parts[1];
        } else {
            filename = prompt("Executable > ");
        }
        filename.Trim();
        filename.ToUpper();

        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();

            if (entryName == filename && !files[i].IsDirectory) {
                uint8_t* data = globalFat32Driver->ReadFile(files[i]);
                if (data) {
                    uint_64 progVirtAddr = 0x800000;
                    uint_64 pageCount = (files[i].Size + 4095) / 4096;
                    for (uint_64 p = 0; p < pageCount; p++) {
                        GlobalPageTableManager->MapMemory(
                            (void*)(progVirtAddr + (p * 4096)), 
                            (void*)((uint_64)data + (p * 4096)), 
                            true
                        );
                    }
                    
                    void* volatile userStackPhys = aligned_alloc(4096, 16384);
                    uint_64 stackVirtAddr = 0x900000;
                    for (uint_64 p = 0; p < 5; p++) {
                        GlobalPageTableManager->MapMemory(
                            (void*)(stackVirtAddr + (p * 4096)),
                            (void*)((uint_64)userStackPhys + (p * 4096)),
                            true
                        );
                    }
                    uint_64 userRsp = stackVirtAddr + 16384 - 8;

                    uint_64 cr3;
                    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
                    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));

                    GlobalRenderer->Print("Jumping to Ring 3...\n");
                    __asm__ volatile ("mov %%rsp, %0" : "=m"(KernelStackPtr));
                    KernelStackPtr -= 128;

                    if (setjmp(shell_context) == 0) {
                        JumpToUser(progVirtAddr, userRsp);
                    } else {
                        __asm__ volatile ("sti");
                        GlobalRenderer->Print("\nReturned from user program.\n");
                    }
                    
                    free((void*)userStackPhys);
                    free((void*)data);
                }
                return;
            }
        }
        GlobalRenderer->Print("File not found: [");
        GlobalRenderer->Print(filename.c_str());
        GlobalRenderer->Print("]\n");
    } 
    else if (cmd == "CAT") {
        if (!globalFat32Driver) return;
        
        String filename;
        if (parts.size() > 1) {
            filename = parts[1];
        } else {
            filename = prompt("File > ");
        }
        filename.Trim();
        filename.ToUpper();

        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        bool found = false;
        for (int i = 0; i < files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();

            if (entryName == filename && !files[i].IsDirectory) {
                String content = ReadDiskHelper(files[i]);
                if (content.size() > 0) {
                    GlobalRenderer->Print(content.c_str());
                    GlobalRenderer->Print("\n");
                } else {
                    GlobalRenderer->Print("[File is empty]\n");
                }
                found = true;
                break;
            }
        }
        if (!found) {
            GlobalRenderer->Print("Error: File not found.\n");
        }
    }
    else if (cmd == "HELP") {
        GlobalRenderer->Print("Available commands: clear, ls [dir], cd <dir>, cat <file>, run <exe>, help\n");
    } 
    else {
        GlobalRenderer->Print("Unknown command: ");
        GlobalRenderer->Print(cmd.c_str());
        GlobalRenderer->Print("\n");
    }
}

extern "C" void kernel_main(BootInfo* bootInfo) {
    Init(bootInfo);

    currentPath = new String("/"); 
    
    while(1) { 
        String input = prompt(String("adam@OS:") + (*currentPath) + "> ");
        InputMan(input);
    }
}
