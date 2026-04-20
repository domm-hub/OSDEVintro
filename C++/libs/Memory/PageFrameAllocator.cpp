#include "PageFrameAllocator.h"

PageFrameAllocator GlobalAllocator;

void PageFrameAllocator::ReadMemoryMap(MemoryDescriptor* memoryMap, uint_64 memoryMapSize, uint_64 descriptorSize) {
    uint_64 memoryMapEntries = memoryMapSize / descriptorSize;
    void* largestFreeMemSeg = nullptr;
    size_t largestFreeMemSegSize = 0;
    uint_64 totalMemory = 0;

    for (uint_64 i = 0; i < memoryMapEntries; i++) {
        MemoryDescriptor* desc = (MemoryDescriptor*)((uint_64)memoryMap + (i * descriptorSize));
        totalMemory += desc->NumberOfPages * 4096;
        if (desc->Type == 7) { // EfiConventionalMemory
            if (desc->NumberOfPages * 4096 > largestFreeMemSegSize) {
                largestFreeMemSeg = desc->PhysicalStart;
                largestFreeMemSegSize = desc->NumberOfPages * 4096;
            }
        }
    }

    uint_64 bitmapSize = totalMemory / 4096 / 8 + 1;
    InitBitmap(bitmapSize, largestFreeMemSeg);

    // Lock all pages first
    ReservePages(0, totalMemory / 4096);

    // Free usable pages
    for (uint_64 i = 0; i < memoryMapEntries; i++) {
        MemoryDescriptor* desc = (MemoryDescriptor*)((uint_64)memoryMap + (i * descriptorSize));
        if (desc->Type == 7) { // EfiConventionalMemory
            FreePages(desc->PhysicalStart, desc->NumberOfPages);
        }
    }

    // Reserve the pages used by the bitmap itself
    ReservePages(PageBitmap.Buffer, bitmapSize / 4096 + 1);
}

void PageFrameAllocator::InitBitmap(size_t bitmapSize, void* bufferAddress) {
    PageBitmap.Size = bitmapSize;
    PageBitmap.Buffer = (uint_8*)bufferAddress;
    for (size_t i = 0; i < bitmapSize; i++) {
        PageBitmap.Buffer[i] = 0;
    }
}

void PageFrameAllocator::FreePage(void* address) {
    uint_64 index = (uint_64)address / 4096;
    if (PageBitmap[index] == false) return;
    PageBitmap.Set(index, false);
}

void PageFrameAllocator::FreePages(void* address, uint_64 pageCount) {
    for (uint_64 i = 0; i < pageCount; i++) {
        FreePage((void*)((uint_64)address + (i * 4096)));
    }
}

void PageFrameAllocator::ReservePage(void* address) {
    uint_64 index = (uint_64)address / 4096;
    if (PageBitmap[index] == true) return;
    PageBitmap.Set(index, true);
}

void PageFrameAllocator::ReservePages(void* address, uint_64 pageCount) {
    for (uint_64 i = 0; i < pageCount; i++) {
        ReservePage((void*)((uint_64)address + (i * 4096)));
    }
}

void* PageFrameAllocator::RequestPage() {
    for (uint_64 i = 0; i < PageBitmap.Size * 8; i++) {
        if (PageBitmap[i] == true) continue;
        ReservePage((void*)(i * 4096));
        return (void*)(i * 4096);
    }
    return nullptr; // Out of RAM
}

uint_64 totalRAM = 0;
uint_64 usedRAM = 0;
uint_64 reservedRAM = 0;

uint_64 PageFrameAllocator::GetFreeRAM() {
    uint_64 free = 0;
    for (uint_64 i = 0; i < PageBitmap.Size * 8; i++) {
        if (PageBitmap[i] == false) free++;
    }
    return free * 4096;
}

uint_64 PageFrameAllocator::GetUsedRAM() {
    uint_64 used = 0;
    for (uint_64 i = 0; i < PageBitmap.Size * 8; i++) {
        if (PageBitmap[i] == true) used++;
    }
    return used * 4096;
}

uint_64 PageFrameAllocator::GetReservedRAM() {
    // For now simple implementation
    return usedRAM; 
}
