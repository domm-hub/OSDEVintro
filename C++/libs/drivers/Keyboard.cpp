#include "TypeDefs.h"
#include "KBSCodesS1.h"
#include "BasicRenderer.h"
#include "Keyboard.h"
#include "PIT.h"

// Tracking state for modifiers and extended keys
bool LShift = false;
bool RShift = false;
bool isExtended = false;
bool AcceptInput = true;
bool prompt = true;

bool StringCompare(const char* str1, const char* str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) return false;
        str1++;
        str2++;
    }
    return (*str1 == *str2);
}

void ProcessCommand() {
    if (!GlobalRenderer) return;

    uint_32 start = GlobalRenderer->PromptSize;
    uint_32 end = GlobalRenderer->BufferSize;
    
    // Extract string
    char cmd[128] = {0};
    uint_32 len = 0;
    for (uint_32 i = start; i < end && i < start + 127; i++) {
        cmd[len++] = GlobalRenderer->TextBuffer[i];
    }
    cmd[len] = '\0';
    
    GlobalRenderer->NextLine(); 

    if (len == 0) {
        // Do nothing for empty
    } else if (StringCompare(cmd, "clear")) {
        GlobalRenderer->Clear(GlobalRenderer->ClearColor);
    } else if (StringCompare(cmd, "time")) {
        GlobalRenderer->Print("Time since boot: ");
        GlobalRenderer->Print(IntegerToString(PIT::TimeSinceBootMS / 1000));
        GlobalRenderer->Print(".");
        uint_64 ms_frac = (PIT::TimeSinceBootMS % 1000) / 10;
        if (ms_frac < 10) GlobalRenderer->Print("0");
        GlobalRenderer->Print(IntegerToString(ms_frac));
        GlobalRenderer->Print(" s\n");
    } else if (StringCompare(cmd, "help")) {
        GlobalRenderer->Print("Available commands: clear, time, help\n");
    } else {
        GlobalRenderer->Print("Unknown command: ");
        GlobalRenderer->Print(cmd);
        GlobalRenderer->Print("\n");
    }

    GlobalRenderer->Print("adam@OS:/>");
    GlobalRenderer->PromptSize = GlobalRenderer->BufferSize;
}

void KeyboardHandler(uint_8 scanCode, uint_8 chr) {
    if (!AcceptInput) return;
    
    if (scanCode == 0xE0) {
        isExtended = true;
        return;
    }

    // Handle Extended Keys
    if (isExtended && !prompt) {
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
                if (GlobalRenderer) GlobalRenderer->Backspace();
                break;
            case 0x1C: // Enter
                ProcessCommand();
                break;
            default:
                if (chr != 0 || KBSet1::ScanCodeLookupTable[scanCode] != 0) {
                    char c = (LShift || RShift) ? KBSet1::ShiftScanCodeLookupTable[scanCode] : KBSet1::ScanCodeLookupTable[scanCode];
                    if (c != 0) {
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
