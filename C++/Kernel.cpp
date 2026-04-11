#include "headers/BootInfo.h"
#include "headers/BasicRenderer.h"
#include "headers/IDT.h"
#include "headers/Keyboard.h"
#include "headers/IO.h"

// Define these symbols from the embedded font
extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;

extern "C" __attribute__((ms_abi)) void _start(BootInfo* bootInfo) {
    // 1. Set up the PSF font
    PSF1_Header* fontHeader = (PSF1_Header*)&_binary_font_psf_start;
    PSF1_Font font = { fontHeader, (void*)((uint_64)fontHeader + sizeof(PSF1_Header)) };
    bootInfo->font = &font;

    // 2. Initialize the Global Renderer
    BasicRenderer renderer(bootInfo->fb, &font);
    GlobalRenderer = &renderer;

    // 3. Clear screen and print early to confirm entry
    GlobalRenderer->Clear(0x00111111); // Dark grey
    GlobalRenderer->Print("Kernel Entered Successfully!\n", 0xFF00FF00);

    activate_sse();
    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;
    
    GlobalRenderer->Print("SSE and IDT Initialized.\n");
    
    GlobalRenderer->Print("Enabling Interrupts...\n");
    __asm__ volatile ("sti");
    GlobalRenderer->Print("Interrupts Enabled.\n");

    GlobalRenderer->Print("Renderer initialized in grid mode.\n");
    GlobalRenderer->Print("Cursor tracking is now active.\n\n");
    
    GlobalRenderer->Print("Testing newline...\n");
    GlobalRenderer->Print("It works!\n");

    GlobalRenderer->Print("Testing Float: ");
    GlobalRenderer->Print(FloatToString(3.141592, 4));
    GlobalRenderer->Print("\n");

    GlobalRenderer->Print("Deleting char x=5, y=2\n");
    GlobalRenderer->DelChar(5, 2);
    GlobalRenderer->Print("Deleted?\n");
    GlobalRenderer->Print("adam@OS:/>_");

    while(1) { __asm__("hlt"); }
}
