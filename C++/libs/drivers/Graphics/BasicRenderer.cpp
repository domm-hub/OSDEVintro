#include "BasicRenderer.h"
#include "TypeDefs.h"
#include "IO.h"
#include "Memory.h"
#include "Heap.h"

BasicRenderer* GlobalRenderer = nullptr;

// Forward declaration of existing low-level drawing
void PutPixel(Framebuffer* fb, uint_32 x, uint_32 y, uint_32 color);

BasicRenderer::BasicRenderer(Framebuffer* fb, PSF1_Font* font) {
    TargetFramebuffer = fb;
    TargetFont = font;
    CursorPosition = {0, 0};
    Color = 0xFFFFFFFF; // White
    BufferSize = 0;
    PromptSize = 0;
    ClearColor = 0x00111111; // Dark grey default
    CursorDrawn = false;
    Locked = false;

    // Allocate BackBuffer
    BackBuffer = malloc(fb->BufferSize);
    if (BackBuffer) {
        memset(BackBuffer, 0, fb->BufferSize);
    }
}

uint_64 BasicRenderer::GetIndex(uint_32 x, uint_32 y) {
    return x + (y * (TargetFramebuffer->Width / 8));
}

void BasicRenderer::DrawCursor(uint_32 color) {
    if (Locked) return; 
    
    CursorDrawn = true;
    unsigned char* glyph = (unsigned char*)TargetFont->glyphBuffer + ('_' * TargetFont->header->charsize);
    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    for (uint_32 i = 0; i < fontHeight; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                uint_32 px = CursorPosition.X + j;
                uint_32 py = CursorPosition.Y + i;
                if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                    PutPixel(TargetFramebuffer, px, py, color);
                    if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = color;
                }
            }
        }
    }
}

void BasicRenderer::ClearCursor() {
    CursorDrawn = false;
    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    for (uint_32 offY = 0; offY < fontHeight; offY++) {
        for (uint_32 offX = 0; offX < 8; offX++) {
            uint_32 px = CursorPosition.X + offX;
            uint_32 py = CursorPosition.Y + offY;
            if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                PutPixel(TargetFramebuffer, px, py, ClearColor); 
                if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = ClearColor;
            }
        }
    }
}

void BasicRenderer::ToggleCursor() {
    if (Locked) return; 
    if (CursorDrawn) {
        ClearCursor();
    } else {
        DrawCursor(0xFFFFFFFF);
    }
}

void BasicRenderer::PutChar(char c, uint_32 color) {
    ClearCursor();

    if (BufferSize < 2048) {
        TextBuffer[BufferSize] = c;
        ColorBuffer[BufferSize] = color;
        BufferSize++;
    }

    if (c == '\n') {
        CursorPosition.X = 0;
        CursorPosition.Y += TargetFont->header->charsize;
        DrawCursor(0xFFFFFFFF);
        return;
    }

    unsigned char* glyph = (unsigned char*)TargetFont->glyphBuffer + (c * TargetFont->header->charsize);
    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    for (uint_32 i = 0; i < fontHeight; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                uint_32 px = CursorPosition.X + j;
                uint_32 py = CursorPosition.Y + i;
                if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                    PutPixel(TargetFramebuffer, px, py, color);
                    if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = color;
                }
            }
        }
    }

    CursorPosition.X += 8;
    if (CursorPosition.X + 8 > TargetFramebuffer->Width) {
        CursorPosition.X = 0;
        CursorPosition.Y += fontHeight;
    }
    
    DrawCursor(0xFFFFFFFF);
}

void BasicRenderer::Print(const char* str, uint_32 color) {
    while (Locked) { __asm__("pause"); } // Wait for screen to be free
    Locked = true; 
    char* chr = (char*)str;
    while (*chr != '\0') {
        PutChar(*chr, color);
        chr++;
    }
    Locked = false;
}

void BasicRenderer::NextLine() {
    while (Locked) { __asm__("pause"); }
    Locked = true;
    ClearCursor();
    CursorPosition.X = 0;
    CursorPosition.Y += TargetFont->header->charsize;
    DrawCursor(0xFFFFFFFF);
    Locked = false;
}

void BasicRenderer::Clear(uint_32 color) {
    while (Locked) { __asm__("pause"); }
    Locked = true;
    ClearColor = color;
    uint_32* pixelPtr = (uint_32*)TargetFramebuffer->BaseAddress;
    for (uint_32 y = 0; y < TargetFramebuffer->Height; y++) {
        for (uint_32 x = 0; x < TargetFramebuffer->Width; x++) {
            pixelPtr[x + (y * TargetFramebuffer->PixelsPerScanLine)] = color;
        }
    }
    
    // Also clear backbuffer
    if (BackBuffer) {
        uint_32* backPixelPtr = (uint_32*)BackBuffer;
        for (uint_32 y = 0; y < TargetFramebuffer->Height; y++) {
            for (uint_32 x = 0; x < TargetFramebuffer->Width; x++) {
                backPixelPtr[x + (y * TargetFramebuffer->PixelsPerScanLine)] = color;
            }
        }
    }

    CursorPosition = {0, 0};
    BufferSize = 0;
    PromptSize = 0;
    DrawCursor(0xFFFFFFFF);
    Locked = false;
}

void BasicRenderer::SwapBuffers() {
    if (!BackBuffer) return;
    memcpy(TargetFramebuffer->BaseAddress, BackBuffer, TargetFramebuffer->BufferSize);
}

void BasicRenderer::SwapArea(uint_32 x, uint_32 y, uint_32 w, uint_32 h) {
    if (!BackBuffer) return;
    
    uint_32* back = (uint_32*)BackBuffer;
    uint_32* front = (uint_32*)TargetFramebuffer->BaseAddress;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;
    uint_32 width = TargetFramebuffer->Width;
    uint_32 height = TargetFramebuffer->Height;

    for (uint_32 i = 0; i < h; i++) {
        uint_32 dy = y + i;
        if (dy >= height) break;
        for (uint_32 j = 0; j < w; j++) {
            uint_32 dx = x + j;
            if (dx >= width) break;
            front[dx + (dy * stride)] = back[dx + (dy * stride)];
        }
    }
}

void BasicRenderer::Backspace() {
    if (BufferSize <= PromptSize) return;

    while (Locked) { __asm__("pause"); }
    Locked = true;
    ClearCursor();

    uint_32 lastIndex = BufferSize - 1;
    char deletedChar = TextBuffer[lastIndex];
    BufferSize--;

    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    if (deletedChar == '\n') {
        CursorPosition.Y -= fontHeight;
        uint_32 charsOnPrevLine = 0;
        for (int i = (int)BufferSize - 1; i >= 0; i--) {
            if (TextBuffer[i] == '\n') break;
            charsOnPrevLine++;
        }
        CursorPosition.X = charsOnPrevLine * 8;
    } else {
        CursorPosition.X -= 8;
        for (uint_32 offY = 0; offY < fontHeight; offY++) {
            for (uint_32 offX = 0; offX < 8; offX++) {
                uint_32 px = CursorPosition.X + offX;
                uint_32 py = CursorPosition.Y + offY;
                if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                    PutPixel(TargetFramebuffer, px, py, ClearColor); 
                    if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = ClearColor;
                }
            }
        }
    }

    DrawCursor(0xFFFFFFFF);
    Locked = false;
}

// Wrappers for external calls
void BasicRenderer::PutCharCoords(int x, int y, char c, uint_32 color) {
    Locked = true;
    unsigned char* glyph = (unsigned char*)TargetFont->glyphBuffer + (c * TargetFont->header->charsize);
    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    for (uint_32 i = 0; i < fontHeight; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                uint_32 px = (x * 8) + j;
                uint_32 py = (y * fontHeight) + i;
                if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                    PutPixel(TargetFramebuffer, px, py, color);
                    if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = color;
                }
            }
        }
    }
    Locked = false;
}

void BasicRenderer::ChangeVisualCursorPosition(uint_32 ox, uint_32 oy, uint_32 nx, uint_32 ny, uint_32 clr) {
    Locked = true;
    // Redraw what was there before
    uint_64 index = GetIndex(ox, oy);
    if (index < BufferSize) {
        PutCharCoords(ox, oy, TextBuffer[index], ColorBuffer[index]);
    } else {
        // Just clear it
        uint_32 fontHeight = TargetFont->header->charsize;
        uint_32 stride = TargetFramebuffer->PixelsPerScanLine;
        for (uint_32 offY = 0; offY < fontHeight; offY++) {
            for (uint_32 offX = 0; offX < 8; offX++) {
                uint_32 px = (ox * 8) + offX;
                uint_32 py = (oy * fontHeight) + offY;
                if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                    PutPixel(TargetFramebuffer, px, py, ClearColor); 
                    if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = ClearColor;
                }
            }
        }
    }
    // Draw new cursor
    PutCharCoords(nx, ny, '_', 0xFFFFFFFF);
    Locked = false;
}

void BasicRenderer::DelChar(int x, int y, bool keepPos, uint_64 clr) {
    Locked = true;
    uint_64 index = GetIndex(x, y);
    if (index < 2048 && index < BufferSize) {
        TextBuffer[index] = ' ';
        ColorBuffer[index] = (uint_32)clr;
    }

    uint_32 fontHeight = TargetFont->header->charsize;
    uint_32 pixelX = x * 8;
    uint_32 pixelY = y * fontHeight;
    uint_32 stride = TargetFramebuffer->PixelsPerScanLine;

    for (uint_32 offY = 0; offY < fontHeight; offY++) {
        for (uint_32 offX = 0; offX < 8; offX++) {
            uint_32 px = pixelX + offX;
            uint_32 py = pixelY + offY;
            if (px < TargetFramebuffer->Width && py < TargetFramebuffer->Height) {
                PutPixel(TargetFramebuffer, px, py, (uint_32)clr); 
                if (BackBuffer) ((uint_32*)BackBuffer)[px + (py * stride)] = (uint_32)clr;
            }
        }
    }

    if (!keepPos) {
        CursorPosition.X = pixelX;
        CursorPosition.Y = pixelY;
        DrawCursor(0xFFFFFFFF);
    }
    Locked = false;
}

extern "C" void GlobalPutChar(char c, uint_32 color) {
    if (GlobalRenderer != nullptr) {
        // Note: GlobalPutChar is often called from ISRs. 
        // We should skip if locked, but ISRs are high priority.
        // For now, we use the lock.
        if (GlobalRenderer->Locked) return;
        GlobalRenderer->PutChar(c, color);
    }
}
