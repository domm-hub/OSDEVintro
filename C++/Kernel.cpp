#include "TextPrint.h"
#include "IDT.h"
#include "Keyboard.h"
#include "MemoryMap.h"

#include "libs/drivers/IO.cpp"
#include "libs/drivers/TextPrint.cpp"
#include "libs/Sets/KBSCodesS1.cpp"
#include "libs/drivers/Keyboard.cpp"
#include "libs/IDT.cpp"

extern "C" void _start(){
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;
    clearScreen(clr);
    PrintString("--- BEFORE: ---\n", clr); 
    PrintString(FloatToString(-632.7523, 1), clr);
    PrintString("\n---- AFTER ----\n", clr);
    PrintString(IntegerToString(MemoryRegionCount));
}