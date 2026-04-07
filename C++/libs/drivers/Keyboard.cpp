#include "TextPrint.cpp"
#include "KBSCodesS1.hpp"
#include "Keyboard.hpp"

bool LShift = false;
bool RShift = false;
uint_8 LastScanCode;


void StandardKeyboardHandler(uint_8 scanCode, uint_8 chr){
        if (scanCode < 0x80 and chr != 0){
        switch (LShift | RShift){
            case true:
                PrintChar(KBSet1::ShiftScanCodeLookupTable[scanCode]);
                break;
            case false:
                PrintChar(chr);
                break;
        }
    } else {
        switch (scanCode){
            case 0x8E:
                SetCursorPosition(CursorPosition - 1);
                PrintChar(' ');
                SetCursorPosition(CursorPosition - 1);
                break;
            case 0x2A:
                LShift = true;
                break;
            case 0xAA:
                LShift = false;
                break;
            case 0x36:
                RShift = true;
                break;
            case 0x86:
                RShift = false;
                break;
            case 0x9C: // Enter
                PrintString("\n\r");
                break;
            


        }
    }
    LastScanCode = scanCode;
}

void ArrowHandler(uint_8 scanCode){
    switch (scanCode){
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

void KeyboardHandler(uint_8 scanCode, uint_8 chr){
    switch (LastScanCode){
        default:
            StandardKeyboardHandler(scanCode, chr);
            break;
        case 0xE0:
            ArrowHandler(scanCode);
            break;
    }
    LastScanCode = scanCode;
}