#include "TextPrint.h"
#include "IDT.h"
#include "Keyboard.h"
#include "MemoryMap.h"

extern "C" void _start(){
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;
    clearScreen(clr);
    PrintString("--- BEFORE: ---\n", clr); 
    PrintString(FloatToString(-632.7523, 1), clr);
    PrintString("\n---- AFTER ----\n", clr);
}