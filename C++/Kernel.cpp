#include "libs/drivers/TextPrint.cpp"
#include "libs/IDT.cpp"
#include "libs/drivers/Keyboard.cpp"

extern const char Test[];

extern "C" void _start () {
    SetCursorPosition(0);
    InitializeIDT();
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    MainKeyboardHandler = KeyboardHandler;
    clearScreen(clr);
    PrintString(Test, clr);
    float x = 6.7;
    PrintString(IntegerToString(-671234567));
    PrintString("\n");
    

    return ;
}