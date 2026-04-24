#pragma once
#include "TypeDefs.h"

namespace Paging {

    enum PageTableFlags {
        Present = 0,
        ReadWrite = 1,
        UserSuper = 2,
        WriteThrough = 3,
        CacheDisabled = 4,
        Accessed = 5,
        Dirty = 6,
        HugePage = 7,
        Global = 8,
        Custom0 = 9,
        Custom1 = 10,
        Custom2 = 11,
        NX = 63 // No Execute
    };

    struct PageDirectoryEntry {
        uint_64 Value;

        void SetFlag(PageTableFlags flag, bool enabled) {
            uint_64 bit = (uint_64)1 << flag;
            if (enabled) Value |= bit;
            else Value &= ~bit;
        }

        bool GetFlag(PageTableFlags flag) {
            uint_64 bit = (uint_64)1 << flag;
            return (Value & bit) > 0;
        }

        void SetAddress(uint_64 address) {
            // Mask the address to ensure only bits 12-51 are used for the physical address field
            // and preserve the flags in bits 0-11 and 52-63
            uint_64 addressMask = 0x000ffffffffff000;
            address &= addressMask;
            Value &= ~addressMask;
            Value |= address;
        }

        uint_64 GetAddress() {
            return Value & 0x000ffffffffff000;
        }
    };

    struct PageTable {
        PageDirectoryEntry entries[512];
    } __attribute__((aligned(4096)));

    class PageTableManager {
    public:
        PageTable* PML4;
        PageTableManager(PageTable* pml4Address);

        void MapMemory(void* virtualMemory, void* physicalMemory, bool userAccessible = false);
        void IdentityMap(void* address, uint_64 size, bool userAccessible = false);
    };

    extern "C" void InvalidatePage(void* address);

}
