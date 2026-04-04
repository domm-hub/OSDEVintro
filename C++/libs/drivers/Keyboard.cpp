#pragma once

#include "TextPrint.cpp"


bool LeftShiftPressed = false;
bool RightShiftPressed = false;
uint_8 lastscancode;

void StandardKeyboardHandler(uint_8 scancode, uint_8 chr){
        if (chr != 0) {
        switch (LeftShiftPressed | RightShiftPressed){
            case true:
                PrintChar(chr - 32);
                break;
            case false:
                PrintChar(chr);
        }
        
    } else {
        switch (scancode) {
            case 0x0E: // Backspace PRESS
                if (CursorPosition > 0) {
                    SetCursorPosition(CursorPosition - 1);
                    PrintChar(' '); // This clears the char and moves cursor forward
                    SetCursorPosition(CursorPosition - 1); // Move cursor back again
                }
                break;
            case 0x2A:
                LeftShiftPressed = true;
                break;
            case 0xAA:
                LeftShiftPressed = false;
                break;
            case 0x36:
                RightShiftPressed = true;
                break;
            case 0xB6:
                RightShiftPressed = false;
                break;
            case 0x1C: // Enter Press
                break;
            case 0x9C:
                PrintString("\n\r");
                break;
        }
    }
}

void KeyboardHandler0xE0(uint_8 scancode){
    switch (scancode){
        default:
            break;
        case 0x50:
            SetCursorPosition(CursorPosition + VGA_WIDTH);
            break;
        case 0x48:
            SetCursorPosition(CursorPosition - VGA_WIDTH);
            break;
    }
}

void KeyBoardHandler(uint_8 scancode, uint_8 chr) {
    if (lastscancode == 0xE0) {
        KeyboardHandler0xE0(scancode);
    } else {
        StandardKeyboardHandler(scancode, chr);
    }
    lastscancode = scancode;
}