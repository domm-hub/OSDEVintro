#include "BootInfo.h"

// PutPixel: Directly writes a 32-bit color value to the linear framebuffer
__attribute__((no_caller_saved_registers))
void PutPixel(Framebuffer* fb, uint_32 x, uint_32 y, uint_32 color) {
    if (x >= fb->Width || y >= fb->Height) return;
    
    // Pixel offset: (y * stride) + x
    uint_32* pixelPtr = (uint_32*)fb->BaseAddress;
    pixelPtr[x + (y * fb->PixelsPerScanLine)] = color;
}