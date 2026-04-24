#include "Init.h"

// --- Global Paging State ---
Paging::PageTableManager* GlobalPageTableManager = nullptr;

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
uint32_t currentDirCluster = 2; 
String* currentPath = nullptr; 

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

    // Identity map the first 4GB of memory - Supervisor only
    for (uint_64 i = 0; i < 0x100000000; i += 4096) {
        GlobalPageTableManager->MapMemory((void*)i, (void*)i, false);
    }

    // Identity map the Framebuffer
    uint_64 fbSize = (uint_64)bootInfo->fb->BufferSize;
    uint_64 fbBase = (uint_64)bootInfo->fb->BaseAddress;
    GlobalAllocator.ReservePages((void*)fbBase, fbSize / 4096 + 1);
    for (uint_64 i = fbBase; i < fbBase + fbSize; i += 4096) {
        GlobalPageTableManager->MapMemory((void*)i, (void*)i, false);
    }

    // Activate Paging
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

    // Save the current kernel stack pointer so syscalls can find it later
    __asm__ volatile ("mov %%rsp, %0" : "=m"(KernelStackPtr));

    InitializeSyscalls();
    AHCI::Init();

    __asm__ volatile ("sti");

    globalFat32Driver = InitStorage();
    if (globalFat32Driver) currentDirCluster = globalFat32Driver->GetRootCluster();
    GlobalRenderer->Print("Ready.\n");
}

uint64_t read_return_code() {
    uint64_t val;
    asm volatile (
        "mov %%r12, %0"  // Move value from R12 into the variable 'val'
        : "=r"(val)      // Output: "=r" means 'write to this register/variable'
        :                // No inputs
        :                // No clobbers needed for the target register
    );
    return val;
}

void InputMan(String input){
    input.Trim();
    if (input.size() == 0) return;
    
    if (input == "clear") {
        GlobalRenderer->Clear(GlobalRenderer->ClearColor);
    } 
    else if (input == "ls") {
        if (!globalFat32Driver) return;
        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < files.size(); i++) {
            GlobalRenderer->Print(files[i].Name.c_str());
            if (files[i].IsDirectory) GlobalRenderer->Print(" [DIR]\n");
            else {
                GlobalRenderer->Print(" (");
                GlobalRenderer->Print(IntegerToString(files[i].Size));
                GlobalRenderer->Print(" bytes)\n");
            }
        }
    }
    else if (input == "cd") {
        if (!globalFat32Driver) return;
        String dirname = prompt("Dir > ");
        dirname.Trim();
        dirname.ToUpper();

        if (dirname == "..") {
            currentDirCluster = globalFat32Driver->GetRootCluster();
            *currentPath = "/";
            return;
        }
        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();
            if (entryName == dirname && files[i].IsDirectory) {
                currentDirCluster = files[i].FirstCluster;
                if (*currentPath == "/") *currentPath = "/" + files[i].Name;
                else *currentPath = *currentPath + "/" + files[i].Name;
                return;
            }
        }
    }
    else if (input == "run") {
        if (!globalFat32Driver) return;
        String filename = prompt("Executable > ");
        filename.Trim();
        filename.ToUpper();

        Vector<FAT32::File> files = globalFat32Driver->ListDirectory(currentDirCluster);
        for (int i = 0; i < files.size(); i++) {
            String entryName = files[i].Name;
            entryName.ToUpper();

            if (entryName == filename && !files[i].IsDirectory) {
                uint8_t* data = globalFat32Driver->ReadFile(files[i]);
                if (data) {
                    // --- FIXED VIRTUAL LOADING ---
                    uint_64 progVirtAddr = 0x800000;
                    uint_64 pageCount = (files[i].Size + 4095) / 4096;
                    for (uint_64 p = 0; p < pageCount; p++) {
                        GlobalPageTableManager->MapMemory(
                            (void*)(progVirtAddr + (p * 4096)), 
                            (void*)((uint_64)data + (p * 4096)), 
                            true
                        );
                    }
                    
                    void* userStackPhys = aligned_alloc(4096, 16384);
                    uint_64 stackVirtAddr = 0x900000;
                    for (uint_64 p = 0; p < 4; p++) {
                        GlobalPageTableManager->MapMemory(
                            (void*)(stackVirtAddr + (p * 4096)),
                            (void*)((uint_64)userStackPhys + (p * 4096)),
                            true
                        );
                    }
                    uint_64 userRsp = stackVirtAddr + 16384;

                    // --- FORCE TLB FLUSH ---
                    uint_64 cr3;
                    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
                    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));

                    GlobalRenderer->Print("Jumping to Ring 3...\n");
                    
                    if (setjmp(shell_context) == 0) {
                        JumpToUser(progVirtAddr, userRsp);
                    } else {
                        GlobalRenderer->Print("\nReturned from user program.\n");
                    }
                    
                    free(userStackPhys);
                    free(data);
                }
                returnCode = read_return_code();
                GlobalRenderer->Print(String("\n Process exited with returnCode") + IntegerToString(returnCode));
                return;
            }
        }
        GlobalRenderer->Print("File not found: [");
        GlobalRenderer->Print(filename.c_str());
        GlobalRenderer->Print("]\n");
    }
    else if (input == "help") {
        GlobalRenderer->Print("Available commands: clear, ls, cd, run, help\n");
    } 
}