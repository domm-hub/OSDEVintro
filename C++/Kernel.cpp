#include "headers/BootInfo.h"
#include "headers/BasicRenderer.h"
#include "headers/IDT.h"
#include "headers/GDT.h"
#include "headers/Keyboard.h"
#include "headers/IO.h"

// Define these symbols from the embedded font
extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

static BasicRenderer renderer(nullptr, nullptr);

extern "C" void kernel_main(BootInfo* bootInfo) {
    InitializeGDT();

    // 1. Set up the PSF font
    PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
    PSF1_Font font = { fontHeader, (void*)((uint_64)fontHeader + sizeof(PSF1_Header)) };
    bootInfo->font = &font;

    // 2. Initialize the Global Renderer
    renderer = BasicRenderer(bootInfo->fb, &font);
    GlobalRenderer = &renderer;

    // 3. Clear screen and print early to confirm entry
    uint_32 clearclr = 0x00111111;
    GlobalRenderer->Clear(clearclr); // Dark grey
    GlobalRenderer->Print("Kernel Entered Successfully!\n", 0xFF00FF00);
    GlobalRenderer->Print("GDT Initialized.\n");

    activate_sse();
    GlobalRenderer->Print("SSE Initialized.\n");

    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;
    GlobalRenderer->Print("IDT Initialized.\n");
    
    GlobalRenderer->Print("Enabling Interrupts...\n");
    __asm__ volatile ("sti");
    GlobalRenderer->Print("Interrupts Enabled.\n");

    GlobalRenderer->Print("Renderer initialized in grid mode.\n");
    GlobalRenderer->Print("Cursor tracking is now active.\n\n");
    
    GlobalRenderer->Print("Testing Float: ");
    GlobalRenderer->Print(FloatToString(3.141592, 4));
    GlobalRenderer->Print("\n");
    GlobalRenderer->Print("Underscore under x=7, y=9?\n");
    GlobalRenderer->PutCharCoords(6, 10, '_', 0xFF00FF00);
    GlobalRenderer->Print("adam@OS:/>");

    while(1) { __asm__("hlt"); }
}
