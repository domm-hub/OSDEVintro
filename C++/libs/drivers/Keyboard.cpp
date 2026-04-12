#include "TypeDefs.h"
#include "KBSCodesS1.h"
#include "../../headers/BasicRenderer.h"

#include "Keyboard.h"


bool LShift = false;
bool RShift = false;

void KeyboardHandler(uint_8 scanCode, uint_8 chr){

if (chr != 0 && scanCode < 0x80) {
    switch (RShift | LShift){
        case false:
            GlobalPutChar(chr);
        case true:
            GlobalPutChar(KBSet1::ShiftScanCodeLookupTable[scanCode]);
    }
    
} else {
    // Modifier or Key Release Handling
    switch (scanCode) {
        // --- Shift Pressed ---
        case 0x2A: 
            LShift = true;
            break;
        case 0x36: 
            RShift = true;
            break;

        // --- Shift Released ---
        case 0xAA: 
            RShift = true;
            break;
        case 0xB6:
            RShift = false;
            break;
        default:
            break;
    }
}

}
