#include "TypeDefs.h"
#include "KBSCodesS1.h"
#include "BasicRenderer.h"
#include "Keyboard.h"
#include "PIT.h"
#include "str.h"
#include "Multitask.h"

// Tracking state for modifiers and extended keys
bool LShift = false;
bool RShift = false;
bool isExtended = false;
bool AcceptInput = true;
volatile bool EnterPressed = false;
volatile bool isPrompting = false;

String prompt(String prmpt) {
    if (!GlobalRenderer) return String();

    EnterPressed = false; 
    isPrompting = true;
    
    GlobalRenderer->Print(prmpt.c_str());
    // Store where the user's input starts in the buffer
    GlobalRenderer->PromptSize = GlobalRenderer->BufferSize;
    
    while (!EnterPressed) {
        Scheduler::Yield(); 
    }
    
    isPrompting = false;

    uint_32 start = GlobalRenderer->PromptSize;
    uint_32 end = GlobalRenderer->BufferSize;
    
    String input = String();
    // Optimization: Ensure we don't overflow or read garbage
    for (uint_32 i = start; i < end; i++) {
        char c = GlobalRenderer->TextBuffer[i];
        if (c != 0) input.add(c);
    }
    
    GlobalRenderer->NextLine();
    return input;
}

void vm(sint_16 incx, sint_16 incy){
        uint_32 oldX = GlobalRenderer->CursorPosition.X;
        uint_32 oldY = GlobalRenderer->CursorPosition.Y;

        // Update the position
        GlobalRenderer->CursorPosition.X += incx;
        // GlobalRenderer->CursorPosition.Y += incy;

        // Call the function with the correct arguments
        GlobalRenderer->ChangeVisualCursorPosition(oldX, oldY, GlobalRenderer->CursorPosition.X, GlobalRenderer->CursorPosition.Y);
}

void KBArrows(uint_8 scancode){
    switch (scancode){
        case 0x4B:
            vm(1, 0);
            break;
        case 0x4D:
            vm(-1, 0);
            break;
    }
}

void KeyboardHandler(uint_8 scanCode, uint_8 chr) {
    if (!AcceptInput) return;
    
    if (scanCode == 0xE0) {
        isExtended = true;
        return;
    }

    // Handle Extended Keys
    if (isExtended) {
        isExtended = false;
        KBArrows(scanCode);
        return;
    }
    isExtended = false; // Reset if it was set but not handled

    if (scanCode < 0x80) {
        switch (scanCode) {
            case 0x2A: LShift = true; break; 
            case 0x36: RShift = true; break; 
            case 0x0E: // Backspace
                if (GlobalRenderer && isPrompting) GlobalRenderer->Backspace();
                break;
            case 0x1C: // Enter
                if (isPrompting) EnterPressed = true;
                break;
            default:
                if (scanCode < 90) { 
                    char c = (LShift || RShift) ? KBSet1::ShiftScanCodeLookupTable[scanCode] : KBSet1::ScanCodeLookupTable[scanCode];
                    if (c != 0 && isPrompting) {
                        GlobalPutChar(c, 0xFFFFFFFF);
                    }
                }
                break;
        }
    } else {
        switch (scanCode) {
            case 0xAA: LShift = false; break; 
            case 0xB6: RShift = false; break; 
        }
    }
}
