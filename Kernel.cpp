#include "libs/TextPrint.cpp"

extern const char Test[];

extern "C" void _start () {
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    clearScreen(clr);
    PrintString(Test, clr);
    return ;
}