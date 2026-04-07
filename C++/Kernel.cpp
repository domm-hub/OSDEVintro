#include "drivers/TextPrint.hpp"
#include "IDT.hpp"
#include "drivers/Keyboard.hpp"

extern const char Test[];



extern "C" void _start () {
    SetCursorPosition(0);
    InitializeIDT();
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    MainKeyboardHandler = KeyboardHandler;
    clearScreen(clr);
    PrintString(Test, clr);

    return ;
}