#include "TextPrint.h"
#include "IDT.h"
#include "Keyboard.h"
#include "MemoryMap.h"
#include "Heap.h"

void println(const char* str, uint_8 clr = 0x0F, const char* prefix="", const char* suffix=""){
    PrintString(prefix);
    PrintString(str, clr);
    PrintString(suffix);
    PrintString("\n", clr);
}


extern "C" void _start(){
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    InitializeIDT();
    PrintString("Hello Guys!\n");
    MainKeyboardHandler = KeyboardHandler;

    MemoryMapEntry** UsableMemoryMaps= GetUsableMemoryRegions();

    InitializeHeap(0x100000, 0x100000);

    uint_64* TestAddress = (uint_64*)aligned_alloc(0x4000, 0x08);
    println(HexToString((uint_64)TestAddress));


}

