#include "libs/drivers/TextPrint.cpp"
#include "libs/IDT.cpp"

extern const char Test[];

void KeyboardHandler(uint_8 scanCode, uint_8 chr){
    if (scanCode < 0x80 and chr != 0){
        PrintChar(chr);
    } else {
        switch (scanCode){
            case 0x8E:
                SetCursorPosition(CursorPosition - 1);
                PrintChar(' ');
                SetCursorPosition(CursorPosition - 1);

        }
    }
}

extern "C" void _start () {
    SetCursorPosition(0);
    InitializeIDT();
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    MainKeyboardHandler = KeyboardHandler;
    clearScreen(clr);
    PrintString(Test, clr);

    return ;
}