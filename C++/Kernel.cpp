#include "libs/drivers/TextPrint.cpp"
#include "libs/IDT.cpp"

extern const char Test[];

extern "C" void _start () {
    SetCursorPosition(0);
    InitializeIDT();
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    clearScreen(clr);
    PrintString(Test, clr);

    return ;
}