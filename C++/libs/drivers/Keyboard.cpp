#include "TypeDefs.h"
#include "KBSCodesS1.h"
#include "BasicRenderer.h"
#include "Keyboard.h"
#include "PIT.h"
#include "str.h"

// Tracking state for modifiers and extended keys
bool LShift = false;
bool RShift = false;
bool isExtended = false;
bool AcceptInput = true;
bool isPrompting = false; // Flag to indicate if we are waiting for input
volatile bool EnterPressed = false;

String prompt(String prmpt) {
    if (!GlobalRenderer) return String();

    GlobalRenderer->Print(prmpt.c_str());
    GlobalRenderer->PromptSize = GlobalRenderer->BufferSize;
    
    EnterPressed = false;
    isPrompting = true;
    
    while (!EnterPressed) {
        __asm__ volatile ("hlt");
    }
    
    isPrompting = false;

    uint_32 start = GlobalRenderer->PromptSize;
    uint_32 end = GlobalRenderer->BufferSize;
    
    String input = String();
    for (uint_32 i = start; i < end; i++) {
        input.add(GlobalRenderer->TextBuffer[i]);
    }
    
    GlobalRenderer->NextLine(); 
    
    return input;
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
        // Skipping arrow key logic for now to keep the shell simple
        return;
    }
    isExtended = false; // Reset if it was set but not handled

    if (scanCode < 0x80) {
        switch (scanCode) {
            case 0x2A: LShift = true; break; // Left Shift
            case 0x36: RShift = true; break; // Right Shift
            case 0x0E: // Backspace
                if (GlobalRenderer && isPrompting) GlobalRenderer->Backspace();
                break;
            case 0x1C: // Enter
                if (isPrompting) EnterPressed = true;
                break;
            default:
                if (chr != 0 || KBSet1::ScanCodeLookupTable[scanCode] != 0) {
                    char c = (LShift || RShift) ? KBSet1::ShiftScanCodeLookupTable[scanCode] : KBSet1::ScanCodeLookupTable[scanCode];
                    if (c != 0 && isPrompting) {
                        GlobalPutChar(c, 0xFFFFFFFF);
                    }
                }
                break;
        }
    } else {
        switch (scanCode) {
            case 0xAA: LShift = false; break; // Left Shift Release
            case 0xB6: RShift = false; break; // Right Shift Release
        }
    }
}
