#include "headers/BootInfo.h"
#include "headers/BasicRenderer.h"
#include "headers/IDT.h"
#include "headers/GDT.h"
#include "headers/Keyboard.h"
#include "headers/IO.h"
#include "headers/Heap.h"
#include "headers/PIT.h"
#include "str.h"
#include "fat32.h"
#include "Vector.h"
#include "headers/AHCI.h"

// Define these symbols from the embedded font
extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

extern "C" int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) {
    return 0;
}
void* __dso_handle = (void*)0;

// Operator new for BasicRenderer and Vector
void* operator new(unsigned long size) {
    return malloc(size);
}

void* operator new[](unsigned long size) {
    return malloc(size);
}

void operator delete(void* p) {
    free(p);
}

void operator delete(void* p, unsigned long size) {
    free(p);
}

void operator delete[](void* p) {
    free(p);
}

void operator delete[](void* p, unsigned long size) {
    free(p);
}

static Framebuffer kernel_fb;
static PSF1_Font kernel_font;

bool DiskReadWrapper(uint64_t lba, uint32_t count, void* buffer) {
    if (AHCI::GlobalAHCIDriver != nullptr && AHCI::GlobalAHCIDriver->portCount > 0) {
        return AHCI::GlobalAHCIDriver->ports[0]->Read(lba, count, buffer);
    }
    return false;
}

FAT32::Driver* globalFat32Driver = nullptr;

extern "C" void kernel_main(BootInfo* bootInfo) {
    // 1. Initialize GDT immediately for safety
    InitializeGDT();

    // 2. Copy data to stable memory
    kernel_fb = *(bootInfo->fb);
    PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
    kernel_font.header = fontHeader;
    kernel_font.glyphBuffer = (void*)((uint_64)fontHeader + sizeof(PSF1_Header));

    // 3. Simple direct screen test (draw a red pixel at 10,10)
    // This confirms FB address and memory access work without renderer/heap
    uint_32* fb_ptr = (uint_32*)kernel_fb.BaseAddress;
    fb_ptr[10 + (10 * kernel_fb.PixelsPerScanLine)] = 0x00FF0000;

    // 4. Initialize Heap
    // We use a safe region. For a simple demo, we can pick a high address 
    // or look at the memory map. 0x2000000 (32MB) is usually safe.
    InitializeHeap(0x2000000, 0x1000000); // 16MB heap

    // 5. Initialize the Global Renderer manually (avoids static constructor issues)
    GlobalRenderer = new BasicRenderer(&kernel_fb, &kernel_font);

    // 6. Clear screen and print
    uint_32 clearclr = 0x00111111;
    GlobalRenderer->Clear(clearclr); // Dark grey
    GlobalRenderer->Print("Kernel Entered Successfully!\n", 0xFF00FF00);
    GlobalRenderer->Print("GDT and Heap Initialized.\n");

    activate_sse();
    GlobalRenderer->Print("SSE Initialized.\n");

    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;
    GlobalRenderer->Print("IDT Initialized.\n");
    
    PIT::SetFrequency(100);
    GlobalRenderer->Print("\nPIT Initialized (100 Hz).\n");

    AHCI::Init();

    GlobalRenderer->Print("Enabling Interrupts...\n");
    __asm__ volatile ("sti");
    GlobalRenderer->Print("Interrupts Enabled.\n");

    while(1) { 
        String input = prompt("adam@OS:/>");
        
        if (input.size() == 0) {
            continue;
        }
        
        if (input == "clear") {
            GlobalRenderer->Clear(GlobalRenderer->ClearColor);
        } else if (input == "time") {
            GlobalRenderer->Print("Time since boot: ");
            GlobalRenderer->Print(IntegerToString(PIT::TimeSinceBootMS / 1000));
            GlobalRenderer->Print(".");
            uint_64 ms_frac = (PIT::TimeSinceBootMS % 1000) / 10;
            if (ms_frac < 10) GlobalRenderer->Print("0");
            GlobalRenderer->Print(IntegerToString(ms_frac));
            GlobalRenderer->Print(" s\n");
        } else if (input == "fs") {
            if (AHCI::GlobalAHCIDriver != nullptr && AHCI::GlobalAHCIDriver->portCount > 0) {
                globalFat32Driver = new FAT32::Driver(DiskReadWrapper);
                GlobalRenderer->Print("FAT32 Driver Initialized on AHCI Port 0.\n");
            } else {
                GlobalRenderer->Print("Error: No active AHCI drive found to mount.\n");
            }
        } else if (input == "ls") {
            if (globalFat32Driver == nullptr) {
                GlobalRenderer->Print("Error: File system not mounted. Run 'fs' first.\n");
            } else {
                Vector<FAT32::File> files = globalFat32Driver->ListDirectory(globalFat32Driver->bpb.root_cluster);
                if (files.size() == 0) {
                    GlobalRenderer->Print("Directory is empty or error reading.\n");
                } else {
                    for (int i = 0; i < files.size(); i++) {
                        GlobalRenderer->Print(files[i].Name);
                        if (files[i].IsDirectory) {
                            GlobalRenderer->Print(" [DIR]");
                        } else {
                            GlobalRenderer->Print(" (");
                            GlobalRenderer->Print(IntegerToString(files[i].Size));
                            GlobalRenderer->Print(" bytes)");
                        }
                        GlobalRenderer->Print("\n");
                    }
                }
            }
        } else if (input == "help") {
            GlobalRenderer->Print("Available commands: clear, time, fs, ls, help\n");
        } else {
            GlobalRenderer->Print("Unknown command: ");
            GlobalRenderer->Print(input);
            GlobalRenderer->Print("\n");
        }
    }
}
