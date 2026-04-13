#pragma once

#include "TypeDefs.h"

struct MemorySegmentHeader {
    uint_64 MemoryLength;
    MemorySegmentHeader* NextSegment;
    MemorySegmentHeader* PreviousSegment;
    MemorySegmentHeader* NextFreeSegment;
    MemorySegmentHeader* PreviousFreeSegment;
    bool Free;
};

struct AlignedMemorySegmentHeader {
    uint_64 MemorySegmentHeaderAddress : 63;
    bool isAligned : 1;
};

void InitializeHeap(uint_64 HeapAddress, uint_64 HeapLength);

void* malloc(uint_64 size);

void free(void* address);

void* calloc(uint_64 size);

void* realloc(void* address, uint_64 newSize);

void* aligned_alloc(uint_64 alignment, uint_64 size);

inline void* operator new(size_t, void* p) throw() { return p; }
inline void* operator new[](size_t, void* p) throw() { return p; }

