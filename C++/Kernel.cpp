#include "TextPrint.h"
#include "IDT.h"
#include "Keyboard.h"
#include "MemoryMap.h"
#include "Heap.h"

void println(const char* str, uint_8 clr = 0x0F){
    PrintString(str, clr);
    PrintString("\n", clr);
}


extern "C" void _start(){
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    InitializeIDT();
    PrintString("Hello Guys!\n");
    MainKeyboardHandler = KeyboardHandler;

    MemoryMapEntry** UsableMemoryMaps= GetUsableMemoryRegions();

    InitializeHeap(0x100000, 0x100000);
    void* testmem = malloc(0x10);
    void* testmem2 = malloc(0x10);
    void* testmem3 = malloc(0x10);


    PrintString("0x");
    println(HexToString((uint_64)testmem));
    println(HexToString((uint_64)testmem2));
    println(HexToString((uint_64)testmem3));



}

