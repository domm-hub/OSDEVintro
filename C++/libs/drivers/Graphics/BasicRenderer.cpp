#include "BasicRenderer.h"
#include "TypeDefs.h"
#include "IO.h"

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
}

uint_64 BasicRenderer::GetIndex(uint_32 x, uint_32 y) {
    return x + (y * (TargetFramebuffer->Width / 8));
}

void BasicRenderer::DrawCursor(uint_32 color) {
    // Only draw underscore if we are NOT currently locked by a print operation
    if (Locked) return; 
    
    CursorDrawn = true;
    unsigned char* glyph = (unsigned char*)TargetFont->glyphBuffer + ('_' * TargetFont->header->charsize);
    for (uint_32 i = 0; i < TargetFont->header->charsize; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                PutPixel(TargetFramebuffer, CursorPosition.X + j, CursorPosition.Y + i, color);
            }
        }
    }
}

void BasicRenderer::ClearCursor() {
    // Force clear without checking lock (used internally by PutChar)
    CursorDrawn = false;
    uint_32 fontHeight = TargetFont->header->charsize;
    for (uint_32 offY = 0; offY < fontHeight; offY++) {
        for (uint_32 offX = 0; offX < 8; offX++) {
            PutPixel(TargetFramebuffer, CursorPosition.X + offX, CursorPosition.Y + offY, ClearColor); 
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
    // This is an internal function, the lock should be handled by the caller (Print/Backspace/etc)
    
    // 1. Clear existing cursor
    ClearCursor();

    // 2. Buffer character
    if (BufferSize < 2048) {
        TextBuffer[BufferSize] = c;
        ColorBuffer[BufferSize] = color;
        BufferSize++;
    }

    // 3. Handle Newline
    if (c == '\n') {
        CursorPosition.X = 0;
        CursorPosition.Y += TargetFont->header->charsize;
        DrawCursor(0xFFFFFFFF);
        // Note: PutChar is internal, lock handled by Print/etc.
        // If we return here, the caller must release the lock.
        return;
    }

    // 4. Draw glyph
    unsigned char* glyph = (unsigned char*)TargetFont->glyphBuffer + (c * TargetFont->header->charsize);
    for (uint_32 i = 0; i < TargetFont->header->charsize; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                PutPixel(TargetFramebuffer, CursorPosition.X + j, CursorPosition.Y + i, color);
            }
        }
    }

    // 5. Advance cursor
    CursorPosition.X += 8;
    if (CursorPosition.X + 8 > TargetFramebuffer->Width) {
        CursorPosition.X = 0;
        CursorPosition.Y += TargetFont->header->charsize;
    }
    
    // 6. Restore cursor at new position
    DrawCursor(0xFFFFFFFF);
}

void BasicRenderer::Print(const char* str, uint_32 color) {
    Locked = true; // Hold lock for the duration of the entire string
    char* chr = (char*)str;
    while (*chr != '\0') {
        PutChar(*chr, color);
        chr++;
    }
    Locked = false;
}

void BasicRenderer::NextLine() {
    Locked = true;
    ClearCursor();
    CursorPosition.X = 0;
    CursorPosition.Y += TargetFont->header->charsize;
    DrawCursor(0xFFFFFFFF);
    Locked = false;
}

void BasicRenderer::Clear(uint_32 color) {
    Locked = true;
    ClearColor = color;
    uint_32* pixelPtr = (uint_32*)TargetFramebuffer->BaseAddress;
    for (uint_32 y = 0; y < TargetFramebuffer->Height; y++) {
        for (uint_32 x = 0; x < TargetFramebuffer->Width; x++) {
            pixelPtr[x + (y * TargetFramebuffer->PixelsPerScanLine)] = color;
        }
    }
    CursorPosition = {0, 0};
    BufferSize = 0;
    PromptSize = 0;
    DrawCursor(0xFFFFFFFF);
    Locked = false;
}

void BasicRenderer::Backspace() {
    if (BufferSize <= PromptSize) return;

    Locked = true;
    ClearCursor();

    uint_32 lastIndex = BufferSize - 1;
    char deletedChar = TextBuffer[lastIndex];
    BufferSize--;

    uint_32 fontHeight = TargetFont->header->charsize;

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
                PutPixel(TargetFramebuffer, CursorPosition.X + offX, CursorPosition.Y + offY, ClearColor); 
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
    for (uint_32 i = 0; i < TargetFont->header->charsize; i++) {
        for (uint_32 j = 0; j < 8; j++) {
            if (glyph[i] & (0x80 >> j)) {
                PutPixel(TargetFramebuffer, (x*8) + j, (y * TargetFont->header->charsize) + i, color);
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
        for (uint_32 offY = 0; offY < fontHeight; offY++) {
            for (uint_32 offX = 0; offX < 8; offX++) {
                PutPixel(TargetFramebuffer, (ox*8) + offX, (oy*fontHeight) + offY, ClearColor); 
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

    for (uint_32 offY = 0; offY < fontHeight; offY++) {
        for (uint_32 offX = 0; offX < 8; offX++) {
            PutPixel(TargetFramebuffer, pixelX + offX, pixelY + offY, (uint_32)clr); 
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
