#include "Paging.h"
#include "PageFrameAllocator.h"
#include "Memory.h"

namespace Paging {

    PageTableManager::PageTableManager(PageTable* pml4Address) {
        PML4 = pml4Address;
    }

    void PageTableManager::MapMemory(void* virtualMemory, void* physicalMemory, bool userAccessible) {
        uint_64 index4 = ((uint_64)virtualMemory >> 39) & 0x1ff;
        uint_64 index3 = ((uint_64)virtualMemory >> 30) & 0x1ff;
        uint_64 index2 = ((uint_64)virtualMemory >> 21) & 0x1ff;
        uint_64 index1 = ((uint_64)virtualMemory >> 12) & 0x1ff;

        PageDirectoryEntry PDE;

        // Level 4 (PML4)
        PDE = PML4->entries[index4];
        PageTable* PDP;
        if (!PDE.GetFlag(Present)) {
            PDP = (PageTable*)GlobalAllocator.RequestPage();
            memset(PDP, 0, 4096);
            PDE.SetAddress((uint_64)PDP >> 12);
            PDE.SetFlag(Present, true);
            PDE.SetFlag(ReadWrite, true);
            if (userAccessible) PDE.SetFlag(UserSuper, true);
            PML4->entries[index4] = PDE;
        } else {
            if (userAccessible && !PDE.GetFlag(UserSuper)) {
                PDE.SetFlag(UserSuper, true);
                PML4->entries[index4] = PDE;
            }
            PDP = (PageTable*)((uint_64)PDE.GetAddress() << 12);
        }

        // Level 3 (PDPT)
        PDE = PDP->entries[index3];
        PageTable* PD;
        if (!PDE.GetFlag(Present)) {
            PD = (PageTable*)GlobalAllocator.RequestPage();
            memset(PD, 0, 4096);
            PDE.SetAddress((uint_64)PD >> 12);
            PDE.SetFlag(Present, true);
            PDE.SetFlag(ReadWrite, true);
            if (userAccessible) PDE.SetFlag(UserSuper, true);
            PDP->entries[index3] = PDE;
        } else {
            if (userAccessible && !PDE.GetFlag(UserSuper)) {
                PDE.SetFlag(UserSuper, true);
                PDP->entries[index3] = PDE;
            }
            PD = (PageTable*)((uint_64)PDE.GetAddress() << 12);
        }

        // Level 2 (Page Directory)
        PDE = PD->entries[index2];
        PageTable* PT;
        if (!PDE.GetFlag(Present)) {
            PT = (PageTable*)GlobalAllocator.RequestPage();
            memset(PT, 0, 4096);
            PDE.SetAddress((uint_64)PT >> 12);
            PDE.SetFlag(Present, true);
            PDE.SetFlag(ReadWrite, true);
            if (userAccessible) PDE.SetFlag(UserSuper, true);
            PD->entries[index2] = PDE;
        } else {
            if (userAccessible && !PDE.GetFlag(UserSuper)) {
                PDE.SetFlag(UserSuper, true);
                PD->entries[index2] = PDE;
            }
            PT = (PageTable*)((uint_64)PDE.GetAddress() << 12);
        }

        // Level 1 (Page Table)
        PDE = PT->entries[index1];
        PDE.SetAddress((uint_64)physicalMemory >> 12);
        PDE.SetFlag(Present, true);
        PDE.SetFlag(ReadWrite, true);
        if (userAccessible) PDE.SetFlag(UserSuper, true);
        PT->entries[index1] = PDE;

        InvalidatePage(virtualMemory);
    }

    void PageTableManager::IdentityMap(void* address, uint_64 size, bool userAccessible) {
        uint_64 startPage = (uint_64)address / 4096;
        uint_64 endPage = ((uint_64)address + size - 1) / 4096;
        
        for (uint_64 p = startPage; p <= endPage; p++) {
            void* addr = (void*)(p * 4096);
            MapMemory(addr, addr, userAccessible);
        }
    }

}
