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
#include "Mouse.h"
#include "Syscalls.h"
#include "Memory.h"
#include "PageFrameAllocator.h"
#include "Paging.h"
#include "Multitask.h"

// --- Global Kernel State ---
static Framebuffer kernel_fb;
static PSF1_Font kernel_font;

Paging::PageTableManager* GlobalPageTableManager = nullptr;
// Static buffers for core structures
static uint8_t gptm_buffer[sizeof(Paging::PageTableManager)];
static uint8_t renderer_buffer[sizeof(BasicRenderer)];
uint8_t kernel_stack[65536] __attribute__((aligned(16)));

extern "C" uint8_t _binary_font_psf_start;
extern "C" uint64_t KernelStackPtr;
extern "C" void JumpToUser(uint_64 rip, uint_64 rsp);

extern Vector<String> split(String text, char delimiter);
extern volatile bool EnterPressed;

AHCI::Port* activePort = nullptr;

bool DiskReadWrapper(uint64_t lba, uint32_t count, void* buffer) {
    if (activePort) return activePort->Read(lba, count, buffer);
    return false;
}

FAT32::Driver* globalFat32Driver = nullptr;
uint32_t currentDirCluster = 2;
String* currentPath = nullptr;

FAT32::Driver* InitStorage() {
    if (!AHCI::GlobalAHCIDriver || AHCI::GlobalAHCIDriver->portCount == 0) return nullptr;
    
    uint8_t* sector0 = (uint8_t*)malloc(512);
    if (!sector0) return nullptr;

    for (int p = 0; p < AHCI::GlobalAHCIDriver->portCount; p++) {
        activePort = AHCI::GlobalAHCIDriver->ports[p];
        if (!activePort || !activePort->hbaPort) continue;

        if (!DiskReadWrapper(0, 1, sector0)) continue;
        
        bool sig52 = (sector0[0x52] == 'F' && sector0[0x53] == 'A' && sector0[0x54] == 'T');
        bool sig36 = (sector0[0x36] == 'F' && sector0[0x37] == 'A' && sector0[0x38] == 'T');
        if (sig52 || sig36) {
            FAT32::Driver* driver = new (malloc(sizeof(FAT32::Driver))) FAT32::Driver(DiskReadWrapper, 0);
            free(sector0);
            return driver;
        }

        if (sector0[510] == 0x55 && sector0[511] == 0xAA) {
            MasterBootRecord* mbr = (MasterBootRecord*)sector0;
            for (int i = 0; i < 4; i++) {
                if (mbr->primaryPartition[i].partition_id == 0x0B || mbr->primaryPartition[i].partition_id == 0x0C) {
                    uint32_t offset = mbr->primaryPartition[i].lba_start;
                    if (offset > 0) {
                        FAT32::Driver* driver = new (malloc(sizeof(FAT32::Driver))) FAT32::Driver(DiskReadWrapper, offset);
                        free(sector0);
                        return driver;
                    }
                }
            }
        }
    }
    activePort = nullptr;
    free(sector0);
    return nullptr;
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
        if (!globalFat32Driver) return;
        uint32_t targetCluster = currentDirCluster;
        if (parts.size() > 1) {
            String dirname = parts[1];
            dirname.ToUpper();
            Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
            bool found = false;
            for (int i = 0; i < (int)files.size(); i++) {
                String entryName = files[i].Name;
                entryName.ToUpper();
                if (entryName == dirname && files[i].IsDirectory) { targetCluster = files[i].FirstCluster; found = true; break; }
            }
            if (!found) {
                GlobalRenderer->Print("Directory not found: ");
                GlobalRenderer->Print(dirname.c_str());
                GlobalRenderer->Print("\n");
                return;
            }
        }
        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(targetCluster);
        if (files.size() == 0) {
            GlobalRenderer->Print("(Empty directory)\n");
        }
        for (int i = 0; i < (int)files.size(); i++) {
            GlobalRenderer->Print(files[i].Name.c_str());
            if (files[i].IsDirectory) GlobalRenderer->Print(" [DIR]\n");
            else { GlobalRenderer->Print(" ("); GlobalRenderer->Print(IntegerToString(files[i].Size)); GlobalRenderer->Print(" bytes)\n"); }
        }
    }
    else if (cmd == "CD") {
        if (!globalFat32Driver) return;
        String dirname;
        if (parts.size() > 1) dirname = parts[1];
        else dirname = prompt("Dir > ");
        dirname.Trim(); dirname.ToUpper();
        if (dirname == "/") { currentDirCluster = globalFat32Driver->GetRootCluster(); *currentPath = "/"; return; }
        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < (int)files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();
            if (entryName == dirname && files[i].IsDirectory) {
                currentDirCluster = files[i].FirstCluster;
                if (dirname == "..") {
                    if (*currentPath != "/") {
                        int lastSlash = -1;
                        for (int j = 0; j < (int)currentPath->length(); j++) if ((*currentPath)[j] == '/') lastSlash = j;
                        if (lastSlash == 0) *currentPath = "/";
                        else { String newPath = ""; for (int j = 0; j < lastSlash; j++) newPath.add((*currentPath)[j]); *currentPath = newPath; }
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
        if (parts.size() > 1) filename = parts[1];
        else filename = prompt("Executable > ");
        filename.Trim(); filename.ToUpper();
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
                        GlobalPageTableManager->MapMemory((void*)(progVirtAddr + (p * 4096)), (void*)((uint_64)data + (p * 4096)), true); 
                    }
                    void* userStackPhys = aligned_alloc(4096, 16384);
                    uint_64 stackVirtAddr = 0x900000;
                    for (uint_64 p = 0; p < 4; p++) { 
                        GlobalPageTableManager->MapMemory((void*)(stackVirtAddr + (p * 4096)), (void*)((uint_64)userStackPhys + (p * 4096)), true); 
                    }
                    uint_64 userRsp = stackVirtAddr + 16384 - 16; 
                    uint_64 cr3; __asm__ volatile("mov %%cr3, %0" : "=r"(cr3)); __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));
                    
                    __asm__ volatile ("mov %%rsp, %0" : "=m"(KernelStackPtr));
                    if (setjmp(shell_context) == 0) { JumpToUser(progVirtAddr, userRsp); } 
                    else { __asm__ volatile ("sti"); GlobalRenderer->Print("\nReturned from user program.\n"); }
                    free(userStackPhys); free(data);
                }
                return;
            }
        }
        GlobalRenderer->Print("File not found.\n");
    } 
    else if (cmd == "HELP") {
        GlobalRenderer->Print("Available commands: clear, ls [dir], cd <dir>, run <exe>, help, amouse, dmouse, ping, ip\n");
    }
    else if (cmd == "AMOUSE") {
        Mouse::Active = true;
        Mouse::DrawMouse();
        GlobalRenderer->Print("Mouse activated.\n");
    }
    else if (cmd == "DMOUSE") {
        Mouse::Active = false;
        GlobalRenderer->Print("Mouse deactivated.\n");
    }
    else {
        GlobalRenderer->Print("Unknown command: "); GlobalRenderer->Print(cmd); GlobalRenderer->Print("\n");
    } 
}

extern "C" void kernel_main(BootInfo* bootInfo) {
    if (bootInfo == nullptr || bootInfo->fb == nullptr) {
        while(1) __asm__("hlt");
    }

    // 1. GDT & Memory Mapping Foundation
    InitializeGDT();
    GlobalAllocator.ReadMemoryMap(bootInfo->memoryMap, bootInfo->memoryMapSize, bootInfo->descriptorSize);
    
    // Protect 0-64MB (Kernel region)
    GlobalAllocator.ReservePages(0, 0x4000000 / 4096); 
    // Protect BootInfo and Memory Map
    GlobalAllocator.ReservePages(bootInfo, sizeof(BootInfo) / 4096 + 1);
    GlobalAllocator.ReservePages(bootInfo->memoryMap, bootInfo->memoryMapSize / 4096 + 1);
// Initial Paging setup
Paging::PageTable* pml4 = (Paging::PageTable*)GlobalAllocator.RequestPage();
memset(pml4, 0, 4096);
GlobalPageTableManager = new (gptm_buffer) Paging::PageTableManager(pml4);

uint_64 memMapEntries = bootInfo->memoryMapSize / bootInfo->descriptorSize;
for (uint_64 i = 0; i < memMapEntries; i++){
    MemoryDescriptor* desc = (MemoryDescriptor*)((uint_64)bootInfo->memoryMap + (i * bootInfo->descriptorSize));
    uint_64 size = desc->NumberOfPages * 4096;
    for (uint_64 j = 0; j < size; j += 4096) {
        GlobalPageTableManager->MapMemory((void*)((uint_64)desc->PhysicalStart + j), (void*)((uint_64)desc->PhysicalStart + j), false);
    }
}
// Map Framebuffer
uint_64 fbSize = (uint_64)bootInfo->fb->BufferSize;
uint_64 fbBase = (uint_64)bootInfo->fb->BaseAddress;
GlobalAllocator.ReservePages((void*)fbBase, fbSize / 4096 + 1);
for (uint_64 i = fbBase; i < fbBase + fbSize; i += 4096) {
    GlobalPageTableManager->MapMemory((void*)i, (void*)i, false);
}

// Now switch to the new page table
LoadCR3((uint_64)pml4);

// 2. Heap Initialization

    GlobalAllocator.ReservePages((void*)0x4000000, 0x1000000 / 4096); 
    InitializeHeap(0x4000000, 0x1000000); 

    // 3. Renderer Setup
    kernel_fb = *(bootInfo->fb);
    if (bootInfo->font && bootInfo->font->header) {
        kernel_font = *(bootInfo->font);
    } else {
        PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
        kernel_font.header = fontHeader;
        kernel_font.glyphBuffer = (void*)((uint_64)fontHeader + sizeof(PSF1_Header));
    }
    GlobalRenderer = new (renderer_buffer) BasicRenderer(&kernel_fb, &kernel_font);
    GlobalRenderer->Clear(0x00111111);
    GlobalRenderer->Print("Ready.\n", 0xFF00FF00);

    // 4. Interrupts & Drivers
    InitializeIDT();
    uint_64 kernel_stack;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(kernel_stack));
    GlobalTSS.rsp0 = kernel_stack;
    MainKeyboardHandler = KeyboardHandler;
    PIT::SetFrequency(100);
    __asm__ volatile ("mov %%rsp, %0" : "=m"(KernelStackPtr));
    
    InitializeSyscalls();
    AHCI::Init();
    globalFat32Driver = InitStorage();
    if (globalFat32Driver) currentDirCluster = globalFat32Driver->GetRootCluster();
    
    Scheduler::Initialize();
    
    currentPath = new String("/");
    __asm__ volatile ("sti");

    while(1) { 
        String input = prompt(String("adam@OS:") + (*currentPath) + "> ");
        InputMan(input);
    }
}
