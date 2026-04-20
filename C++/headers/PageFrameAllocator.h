#pragma once
#include "TypeDefs.h"
#include "Bitmap.h"
#include "BootInfo.h"
#include "Memory.h"

class PageFrameAllocator {
public:
    void ReadMemoryMap(MemoryDescriptor* memoryMap, uint_64 memoryMapSize, uint_64 descriptorSize);
    Bitmap PageBitmap;

    void FreePage(void* address);
    void ReservePage(void* address);
    void FreePages(void* address, uint_64 pageCount);
    void ReservePages(void* address, uint_64 pageCount);
    void* RequestPage();

    uint_64 GetFreeRAM();
    uint_64 GetUsedRAM();
    uint_64 GetReservedRAM();

private:
    void InitBitmap(size_t bitmapSize, void* bufferAddress);
};

extern PageFrameAllocator GlobalAllocator;
