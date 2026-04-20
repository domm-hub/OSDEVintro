#pragma once
#include "BootInfo.h"
#include "Vector.h"


typedef struct {
    uint_32 X;
    uint_32 Y;
} Point;

class BasicRenderer {
public:
    uint_32 ColorBuffer[2048]; 
    char TextBuffer[2048];
    
    uint_32 BufferSize;

    BasicRenderer(Framebuffer* fb, PSF1_Font* font);
    
    void Print(const char* str, uint_32 color = 0xFFFFFFFF);

    void PutChar(char c, uint_32 color = 0xFFFFFFFF);

    uint_64 GetIndex(uint_32 x, uint_32 y);

    void Clear(uint_32 color = 0x00000000);

    void DelChar(int x, int y, bool keepPos = true, uint_64 clr = 0x00000000);

    void PutCharCoords(int x, int y, char c, uint_32 color);

    void Backspace();
    
    void Redraw();

    void ChangeVisualCursorPosition(uint_32 ox, uint_32 oy, uint_32 nx, uint_32 ny, uint_32 clr = 0x0);
    
    void DrawCursor(uint_32 color = 0xFFFFFFFF);
    void ClearCursor();
    void ToggleCursor();

    volatile bool Locked;
    uint_32 PromptSize;
    uint_32 ClearColor;
    bool CursorDrawn;
    Point CursorPosition;
    Framebuffer* TargetFramebuffer;
    PSF1_Font* TargetFont;
    uint_32 Color;

    void NextLine();
};

extern BasicRenderer* GlobalRenderer;
