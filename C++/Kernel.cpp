#include "headers/BootInfo.h"
#include "headers/BasicRenderer.h"
#include "headers/IDT.h"
#include "headers/GDT.h"
#include "headers/Keyboard.h"
#include "headers/IO.h"
#include "headers/Heap.h"
#include "headers/PIT.h"
#include "str.h"

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
    GlobalRenderer->Print("Trying Dynamic String\n");

    String str = String();
    GlobalRenderer->Print("Comparing String(HI) and HI\n");
    str = "HI";
    if (str == "HI"){
        GlobalRenderer->Print("Assertion Complete\n\n");
    } else {
        GlobalRenderer->Print("Assertion Failed\n\n");
    }

    GlobalRenderer->Print("Trying to print value String(HI)\n");
    GlobalRenderer->Print(str + "\n");
    GlobalRenderer->Print("Ok...\n Trying adding strings.\n Printing:");
    String str2 = String();
    str2 = " Hello!";
    String str3 = str + str2;
    GlobalRenderer->Print(str3);


    
    PIT::SetFrequency(100);
    GlobalRenderer->Print("PIT Initialized (100 Hz).\n");

    GlobalRenderer->Print("Enabling Interrupts...\n");
    __asm__ volatile ("sti");
    GlobalRenderer->Print("Interrupts Enabled.\n");

    GlobalRenderer->Print("adam@OS:/>");
    GlobalRenderer->PromptSize = GlobalRenderer->BufferSize;

    while(1) { __asm__("hlt"); }
}
