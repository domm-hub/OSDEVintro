#include "Heap.h"
#include "TypeDefs.h"
#include "Memory.h"

MemorySegmentHeader* FirstFreeMemorySegment;

MemorySegmentHeader* GetHeader(void* address) {
    if (!address) return nullptr;
    uint_64* checkPtr = (uint_64*)((uint_64)address - 8);
    // Use a clearer bit mask and check against our AlignedMemorySegmentHeader structure.
    // Bit 63 is our isAligned flag.
    if ((*checkPtr) & (1ULL << 63)) {
        AlignedMemorySegmentHeader* amsh = (AlignedMemorySegmentHeader*)checkPtr;
        return (MemorySegmentHeader*)amsh->MemorySegmentHeaderAddress;
    }
    return (MemorySegmentHeader*)((uint_64)address - sizeof(MemorySegmentHeader));
}

void InitializeHeap(uint_64 HeapAddress, uint_64 HeapLength){
    memset((void*)HeapAddress, 0, sizeof(MemorySegmentHeader));
    FirstFreeMemorySegment = (MemorySegmentHeader*)HeapAddress;
    FirstFreeMemorySegment->MemoryLength = HeapLength - sizeof(MemorySegmentHeader);
    FirstFreeMemorySegment->NextSegment = nullptr;
    FirstFreeMemorySegment->PreviousSegment = nullptr;
    FirstFreeMemorySegment->NextFreeSegment = nullptr;
    FirstFreeMemorySegment->PreviousFreeSegment = nullptr;
    FirstFreeMemorySegment->Free = true;
}

void* malloc(uint_64 size) {
    if (size == 0) return nullptr;

    // 1. Align to 16 bytes for SSE and general stability
    if (size % 16 != 0) {
        size += (16 - (size % 16));
    }

    MemorySegmentHeader* current = FirstFreeMemorySegment;

    while (current != nullptr) {
        if (current->Free && current->MemoryLength >= size) {
            
            // Need enough room for: Allocated Size + Next Header + Aligned Check Padding (8)
            uint_64 minSplitSize = size + sizeof(MemorySegmentHeader) + 16;
            
            if (current->MemoryLength >= minSplitSize) {
                MemorySegmentHeader* newHeader = (MemorySegmentHeader*)((uint_64)current + sizeof(MemorySegmentHeader) + size);
                memset(newHeader, 0, sizeof(MemorySegmentHeader));
                
                newHeader->MemoryLength = current->MemoryLength - size - sizeof(MemorySegmentHeader);
                newHeader->Free = true;
                
                newHeader->NextSegment = current->NextSegment;
                newHeader->PreviousSegment = current;
                if (current->NextSegment) current->NextSegment->PreviousSegment = newHeader;
                current->NextSegment = newHeader;

                newHeader->NextFreeSegment = current->NextFreeSegment;
                newHeader->PreviousFreeSegment = current->PreviousFreeSegment;
                if (current->NextFreeSegment) current->NextFreeSegment->PreviousFreeSegment = newHeader;
                if (current->PreviousFreeSegment) current->PreviousFreeSegment->NextFreeSegment = newHeader;
                
                if (FirstFreeMemorySegment == current) FirstFreeMemorySegment = newHeader;

                current->MemoryLength = size;
            } else {
                // Remove from free list
                if (current->PreviousFreeSegment) current->PreviousFreeSegment->NextFreeSegment = current->NextFreeSegment;
                if (current->NextFreeSegment) current->NextFreeSegment->PreviousFreeSegment = current->PreviousFreeSegment;
                if (FirstFreeMemorySegment == current) FirstFreeMemorySegment = current->NextFreeSegment;
            }

            current->Free = false;
            current->NextFreeSegment = nullptr;
            current->PreviousFreeSegment = nullptr;

            return (void*)((uint_64)current + sizeof(MemorySegmentHeader));
        }
        current = current->NextFreeSegment;
    }

    return nullptr;
}

void CombineFreeSegments(MemorySegmentHeader* a, MemorySegmentHeader* b) {
    if (a == nullptr || b == nullptr) return;
    
    if (a > b) {
        MemorySegmentHeader* temp = a;
        a = b;
        b = temp;
    }

    a->MemoryLength += b->MemoryLength + sizeof(MemorySegmentHeader);
    
    a->NextSegment = b->NextSegment;
    if (b->NextSegment != nullptr) {
        b->NextSegment->PreviousSegment = a;
    }

    if (b->PreviousFreeSegment != nullptr) {
        b->PreviousFreeSegment->NextFreeSegment = b->NextFreeSegment;
    }
    if (b->NextFreeSegment != nullptr) {
        b->NextFreeSegment->PreviousFreeSegment = b->PreviousFreeSegment;
    }
    
    if (FirstFreeMemorySegment == b) {
        FirstFreeMemorySegment = a; 
    }
    
    b->NextSegment = nullptr;
    b->PreviousSegment = nullptr;
    b->NextFreeSegment = nullptr;
    b->PreviousFreeSegment = nullptr;
    b->Free = false;
}

void free(void* address) {
    if (address == nullptr) return;

    MemorySegmentHeader* current = GetHeader(address);
    
    if (current->Free) return;

    current->Free = true;
    current->NextFreeSegment = nullptr;
    current->PreviousFreeSegment = nullptr;

    if (FirstFreeMemorySegment != nullptr) {
        FirstFreeMemorySegment->PreviousFreeSegment = current;
    }
    current->NextFreeSegment = FirstFreeMemorySegment;
    FirstFreeMemorySegment = current;

    if (current->NextSegment != nullptr && current->NextSegment->Free) {
        CombineFreeSegments(current, current->NextSegment);
    }

    if (current->PreviousSegment != nullptr && current->PreviousSegment->Free) {
        CombineFreeSegments(current->PreviousSegment, current);
    }
}

void* calloc(uint_64 size) {
    void* ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void* calloc(uint_64 num, uint_64 size) {
    return calloc(num * size);
}

void* realloc(void* address, uint_64 newSize) {
    if (address == nullptr) return malloc(newSize);
    if (newSize == 0) {
        free(address);
        return nullptr;
    }

    MemorySegmentHeader* oldHeader = GetHeader(address);
    uint_64 oldSize = oldHeader->MemoryLength;

    if (newSize <= oldSize) {
        return address;
    }

    void* newMem = malloc(newSize);
    if (newMem) {
        memcpy(newMem, address, oldSize);
        free(address);
    }
    return newMem;
}

void* aligned_alloc(uint_64 alignment, uint_64 size) {
    if (alignment < 8) alignment = 8;
    if ((alignment & (alignment - 1)) != 0) return nullptr; 

    uint_64 totalSize = size + alignment; 
    
    void* rawPtr = malloc(totalSize);
    if (!rawPtr) return nullptr;

    uint_64 addr = (uint_64)rawPtr;
    uint_64 alignedAddr = (addr + alignment) & ~(alignment - 1);

    // Store alignment info in the 8 bytes before the aligned address
    AlignedMemorySegmentHeader* amsh = (AlignedMemorySegmentHeader*)(alignedAddr - 8);
    amsh->MemorySegmentHeaderAddress = (uint_64)rawPtr - sizeof(MemorySegmentHeader);
    amsh->isAligned = true;

    return (void*)alignedAddr;
}