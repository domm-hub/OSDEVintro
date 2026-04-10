#include "TextPrint.h"
#include "IDT.h"
#include "Keyboard.h"
#include "MemoryMap.h"
#include "Heap.h"

extern "C" void _start(){
    uint_8 clr = BACKGROUND_BLACK | FOREGROUND_WHITE;
    InitializeIDT();
    MainKeyboardHandler = KeyboardHandler;

    MemoryMapEntry** UsableMemoryMaps= GetUsableMemoryRegions();

    for (uint_8 i = 0; i < UsableMemoryRegionsCount; i++){
        MemoryMapEntry* memMap = UsableMemoryMaps[i];
        PrintMemoryMap(memMap, CursorPosition);
    }

}

