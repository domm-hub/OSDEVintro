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
    float x = 6.7f;
    int y = (int)x;

    if (y == 6) {
        PrintString("SSE IS WORKING!", clr);
    } else if (y == 0) {
        PrintString("SSE RETURNED ZERO", clr);
    } else {
        PrintString("TOTAL CHAOS", clr);
    }

    return ;
}