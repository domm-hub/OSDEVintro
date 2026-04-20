#pragma once
#include "TypeDefs.h"

namespace PCI {
    struct PCIDeviceHeader {
        uint_16 VendorID;
        uint_16 DeviceID;
        uint_16 Command;
        uint_16 Status;
        uint_8 RevisionID;
        uint_8 ProgIF;
        uint_8 Subclass;
        uint_8 Class;
        uint_8 CacheLineSize;
        uint_8 LatencyTimer;
        uint_8 HeaderType;
        uint_8 BIST;
    };

    uint_32 ReadDWord(uint_16 bus, uint_16 slot, uint_16 func, uint_16 offset);
    void WriteDWord(uint_16 bus, uint_16 slot, uint_16 func, uint_16 offset, uint_32 value);
    uint_16 GetVendorID(uint_16 bus, uint_16 slot, uint_16 func);
    uint_16 GetDeviceID(uint_16 bus, uint_16 slot, uint_16 func);
    uint_8 GetClassId(uint_16 bus, uint_16 slot, uint_16 func);
    uint_8 GetSubclassId(uint_16 bus, uint_16 slot, uint_16 func);
    uint_8 GetProgIF(uint_16 bus, uint_16 slot, uint_16 func);
    uint_32 GetBAR(uint_16 bus, uint_16 slot, uint_16 func, uint_8 bar);
    
    PCIDeviceHeader GetDeviceHeader(uint_16 bus, uint_16 slot, uint_16 func);
    void EnumeratePCI();
}
