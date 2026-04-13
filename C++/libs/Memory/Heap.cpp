#include "Heap.h"
#include "TypeDefs.h"
#include "Memory.h"

MemorySegmentHeader* FirstFreeMemorySegment;

void InitializeHeap(uint_64 HeapAddress, uint_64 HeapLength){
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

    // 1. Align to 8 bytes
    if (size % 8 != 0) {
        size += (8 - (size % 8));
    }

    MemorySegmentHeader* current = FirstFreeMemorySegment;

    while (current != nullptr) {
        if (current->Free && current->MemoryLength >= size) {
            
            // Check if we can split the block
            // We need enough room for the new header + at least 1 byte of data (aligned to 8)
            if (current->MemoryLength > size + sizeof(MemorySegmentHeader)) {
                
                // Create new header immediately after the allocated space
                MemorySegmentHeader* newHeader = (MemorySegmentHeader*)((uint_64)current + sizeof(MemorySegmentHeader) + size);
                
                // Calculate new length
                newHeader->MemoryLength = current->MemoryLength - size - sizeof(MemorySegmentHeader);
                newHeader->Free = true;
                
                // --- Link into Physical Chain (Next/Prev Segment) ---
                newHeader->NextSegment = current->NextSegment;
                newHeader->PreviousSegment = current;
                
                if (current->NextSegment != nullptr) {
                    current->NextSegment->PreviousSegment = newHeader;
                }
                current->NextSegment = newHeader;

                // --- Link into Free List Chain (Next/Prev Free) ---
                newHeader->NextFreeSegment = current->NextFreeSegment;
                newHeader->PreviousFreeSegment = current->PreviousFreeSegment;
                
                if (current->NextFreeSegment != nullptr) {
                    current->NextFreeSegment->PreviousFreeSegment = newHeader;
                }
                if (current->PreviousFreeSegment != nullptr) {
                    current->PreviousFreeSegment->NextFreeSegment = newHeader;
                }
                
                // If current was the head of the free list, newHeader is now the head
                if (FirstFreeMemorySegment == current) {
                    FirstFreeMemorySegment = newHeader;
                }

                // Shrink current block
                current->MemoryLength = size;
            } else {
                // Cannot split. Remove current from Free List entirely.
                
                if (current->PreviousFreeSegment != nullptr) {
                    current->PreviousFreeSegment->NextFreeSegment = current->NextFreeSegment;
                }
                if (current->NextFreeSegment != nullptr) {
                    current->NextFreeSegment->PreviousFreeSegment = current->PreviousFreeSegment;
                }
                
                // If current was the head, move head forward
                if (FirstFreeMemorySegment == current) {
                    FirstFreeMemorySegment = current->NextFreeSegment;
                }
            }

            // Mark as Used and Clear Free Pointers to prevent corruption
            current->Free = false;
            current->NextFreeSegment = nullptr;
            current->PreviousFreeSegment = nullptr;

            return (void*)((uint_64)current + sizeof(MemorySegmentHeader));
        }

        // Move to next FREE segment (Optimization: skip used segments in search)
        // Note: In a simple allocator, iterating NextFreeSegment is faster than NextSegment
        current = current->NextFreeSegment;
    }

    return nullptr; // Out of memory
}

void CombineFreeSegments(MemorySegmentHeader* a, MemorySegmentHeader* b) {
    if (a == nullptr || b == nullptr) return;
    
    // Ensure 'a' is the lower address
    if (a > b) {
        MemorySegmentHeader* temp = a;
        a = b;
        b = temp;
    }

    // Merge b into a
    a->MemoryLength += b->MemoryLength + sizeof(MemorySegmentHeader);
    
    // Update Physical Chain
    a->NextSegment = b->NextSegment;
    if (b->NextSegment != nullptr) {
        b->NextSegment->PreviousSegment = a;
    }

    // Update Free List Chain for 'a' (keep a's existing free neighbors)
    // But we need to remove 'b' from the free list
    
    if (b->PreviousFreeSegment != nullptr) {
        b->PreviousFreeSegment->NextFreeSegment = b->NextFreeSegment;
    }
    if (b->NextFreeSegment != nullptr) {
        b->NextFreeSegment->PreviousFreeSegment = b->PreviousFreeSegment;
    }
    
    // If b was the head, a becomes head (if a isn't already in list properly)
    if (FirstFreeMemorySegment == b) {
        FirstFreeMemorySegment = a; 
    }
    
    // Clean up b pointers just in case
    b->NextSegment = nullptr;
    b->PreviousSegment = nullptr;
    b->NextFreeSegment = nullptr;
    b->PreviousFreeSegment = nullptr;
    b->Free = false; // Technically unused now
}

void free(void* address) {
    if (address == nullptr) return;

    MemorySegmentHeader* current = (MemorySegmentHeader*)((uint_64)address - sizeof(MemorySegmentHeader));
    
    // Safety check: Is it already free?
    if (current->Free) return;

    current->Free = true;
    current->NextFreeSegment = nullptr;
    current->PreviousFreeSegment = nullptr;

    // Insert into Free List. 
    // Simple strategy: Add to head of Free List, then try to combine.
    
    if (FirstFreeMemorySegment != nullptr) {
        FirstFreeMemorySegment->PreviousFreeSegment = current;
    }
    current->NextFreeSegment = FirstFreeMemorySegment;
    FirstFreeMemorySegment = current;

    // Try to combine with Next Segment
    if (current->NextSegment != nullptr && current->NextSegment->Free) {
        CombineFreeSegments(current, current->NextSegment);
    }

    // Try to combine with Previous Segment
    // Note: After combining with Next, 'current' might have grown, but its pointer remains valid.
    // However, we must re-fetch the previous segment because 'current' didn't change address.
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

    MemorySegmentHeader* oldHeader = (MemorySegmentHeader*)((uint_64)address - sizeof(MemorySegmentHeader));
    uint_64 oldSize = oldHeader->MemoryLength;

    if (newSize <= oldSize) {
        return address; // Can fit in existing block (could shrink, but ignoring for simplicity)
    }

    void* newMem = malloc(newSize);
    if (newMem) {
        memcpy(newMem, address, oldSize);
        free(address);
    }
    return newMem;
}