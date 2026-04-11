#pragma once

#include "TypeDefs.h"

struct MemoryMapEntry{
    uint_64 BaseAddress;
    uint_64 RegionLength;
    uint_32 RegionType;
    uint_32 ExtendedAttributes;
};

extern MemoryMapEntry* UsableMemoryRegions[10];
extern uint_8 UsableMemoryRegionsCount;

extern uint_8 MemoryRegionCount;

void PrintMemoryMap(MemoryMapEntry* memoryMap, uint_16 position);

MemoryMapEntry** GetUsableMemoryRegions();
