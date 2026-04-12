#include "Heap.h"
#include "TypeDefs.h"
#include "Memory.h"

MemorySegmentHeader* FirstFreeMemorySegment;

void InitializeHeap(uint_64 HeapAddress, uint_64 HeapLength){
    FirstFreeMemorySegment = (MemorySegmentHeader*)HeapAddress;
    FirstFreeMemorySegment->MemoryLength = HeapLength - (sizeof(MemorySegmentHeader));
    FirstFreeMemorySegment->NextSegment = 0;
    FirstFreeMemorySegment->PreviousSegment = 0;
    FirstFreeMemorySegment->NextFreeSegment = 0;
    FirstFreeMemorySegment->PreviousFreeSegment = 0;
    FirstFreeMemorySegment->Free = true;
}

void* malloc(uint_64 size) {
    // 1. Align to 8 bytes (The CPU says thank you)
    uint_64 remainder = size % 8;
    if (remainder != 0) size += (8 - remainder);

    MemorySegmentHeader* current = FirstFreeMemorySegment;

    while (true) {
        // If we hit the end of the line, return 0 (No more memory!)
        if (current == 0) return 0; 

        if (current->MemoryLength >= size) {
            // Can we split it? (Need room for header + at least 8 bytes of data)
            if (current->MemoryLength > size + sizeof(MemorySegmentHeader)) {
                MemorySegmentHeader* newHeader = (MemorySegmentHeader*)((uint_64)current + sizeof(MemorySegmentHeader) + size);
                
                newHeader->Free = true;
                newHeader->MemoryLength = current->MemoryLength - size - sizeof(MemorySegmentHeader);
                
                // Link the new header into the chain
                newHeader->NextSegment = current->NextSegment;
                newHeader->PreviousSegment = current;
                newHeader->NextFreeSegment = current->NextFreeSegment;
                newHeader->PreviousFreeSegment = current->PreviousFreeSegment;

                // Make the current block point to the new one
                current->NextSegment = newHeader;
                current->NextFreeSegment = newHeader;
                current->MemoryLength = size;
            }

            // If we are taking the head of the free list, move the head forward
            if (current == FirstFreeMemorySegment) {
                FirstFreeMemorySegment = current->NextFreeSegment;
            }

            current->Free = false;

            // Update the "Free List" pointers to skip over this now-taken block
            if (current->PreviousFreeSegment != 0) {
                current->PreviousFreeSegment->NextFreeSegment = current->NextFreeSegment;
            }
            if (current->NextFreeSegment != 0) {
                current->NextFreeSegment->PreviousFreeSegment = current->PreviousFreeSegment;
            }
            if (current->PreviousSegment != 0){
                current->PreviousSegment->NextFreeSegment = current->NextFreeSegment;
            }

            if (current->NextSegment != 0){
                current->NextSegment->PreviousFreeSegment = current->PreviousFreeSegment;
            }

            return current + 1; // Hand over the keys to the room!
        }

        // --- THE CRITICAL MOVE ---
        // If the block was too small, walk to the next one!
        current = current->NextFreeSegment;
    }
}

void CombineFreeSegments(MemorySegmentHeader* a, MemorySegmentHeader* b){
    if ((a == 0) | (b == 0)) return;
    if (a < b){
        a->MemoryLength += b->MemoryLength + sizeof(MemorySegmentHeader);
        a->NextSegment = b->NextSegment;
        a->NextFreeSegment = b->NextFreeSegment;
        b->NextSegment->PreviousSegment = a;
        b->NextSegment->PreviousFreeSegment = a;
        b->NextFreeSegment->PreviousFreeSegment = a;

    } else {
        b->MemoryLength += a->MemoryLength + sizeof(MemorySegmentHeader);
        b->NextSegment = a->NextSegment;
        b->NextFreeSegment = a->NextFreeSegment;
        a->NextSegment->PreviousSegment = b;
        a->NextSegment->PreviousFreeSegment = b;
        a->NextFreeSegment->PreviousFreeSegment = b;
    }
}


void free(void* address){
    if (address == nullptr) return;
    MemorySegmentHeader* current;
    AlignedMemorySegmentHeader* ASMH = (AlignedMemorySegmentHeader*)address - 1;
    if (ASMH->isAligned){
        current = (MemorySegmentHeader*)(uint_64)ASMH->MemorySegmentHeaderAddress;
    } else {
        current = ((MemorySegmentHeader*)address) - 1;
    }
    current->Free = true;

    if (current < FirstFreeMemorySegment) FirstFreeMemorySegment = current;

    if (current->NextFreeSegment != 0){
        if (current->NextFreeSegment->PreviousFreeSegment < current){
            current->NextFreeSegment->PreviousFreeSegment = current;

        }
    }
    if (current->PreviousFreeSegment != 0){
        if (current->PreviousFreeSegment->NextFreeSegment > current){
            current->PreviousFreeSegment->NextFreeSegment = current;
        }
    }

    if (current->NextSegment != 0){
        current->NextSegment->PreviousSegment = current;
        if (current->NextSegment->Free){
            CombineFreeSegments(current, current->NextSegment);
        }
    }
    if (current->PreviousSegment != 0){
        current->PreviousSegment->NextSegment = current;
        if (current->PreviousSegment->Free){
            CombineFreeSegments(current, current->PreviousSegment);
        }
    }
}


void* calloc(uint_64 size){
    void* mallocVal = malloc(size);
    if (mallocVal != nullptr) memset(mallocVal, 0, size);
    return mallocVal;
}

void* calloc(uint_64 num, uint_64 size) {
    return (calloc(num*size));
}


void* realloc(void* address, uint_64 newSize){
    if (address == nullptr) return malloc(newSize);
    if (newSize == 0) {
        free(address);
        return nullptr;
    }

    MemorySegmentHeader* oldsegment;
    AlignedMemorySegmentHeader* ASMH = (AlignedMemorySegmentHeader*)address - 1;
    if (ASMH->isAligned){
        oldsegment = (MemorySegmentHeader*)(uint_64)ASMH->MemorySegmentHeaderAddress;
    } else {
        oldsegment = ((MemorySegmentHeader*)address) - 1;
    }

    uint_64 smallerSize = newSize;
    if (oldsegment->MemoryLength < newSize){
        smallerSize = oldsegment->MemoryLength;
    }
    void* newMem = malloc(newSize);
    if (newMem != nullptr) {
        memcpy(newMem, address, smallerSize);
        free(address);
    }
    return newMem;
}


void* aligned_alloc(uint_64 alignment, uint_64 size){
    uint_64 alignmentRemainder = alignment % 8;
    alignment -= alignmentRemainder;
    if (alignmentRemainder != 0) alignment += 8;

    uint_64 sizeRemainder = size % 8;
    alignment -= sizeRemainder;
    if (sizeRemainder != 0) size += 8;

    uint_64 fullsize = size+alignment;
    void* mallocVal = malloc(fullsize);
    uint_64 address = (uint_64)mallocVal;

    uint_64 remainder = address % alignment;
    address -= remainder;
    if (remainder != 0){
        address += alignment;

        AlignedMemorySegmentHeader* ASMH = (AlignedMemorySegmentHeader*)address-1;
        ASMH->isAligned = true;
        ASMH->MemorySegmentHeaderAddress = (uint_64)mallocVal-sizeof(MemorySegmentHeader);
    }
    return (void*)address;
}

// Safely copies memory even if the source and destination overlap
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    if (d == s) {
        return dest;
    }

    if (d < s) {
        // Destination is before source: Copy forward (like memcpy)
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        // Destination is after source: Overlap! Copy backward
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}