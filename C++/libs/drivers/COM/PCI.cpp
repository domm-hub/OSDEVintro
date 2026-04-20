#include "PCI.h"
#include "IO.h"
#include "BasicRenderer.h"

namespace PCI {
    
    // Renamed from ReadWord to ReadDWord to reflect 32-bit read
    uint_32 ReadDWord(uint_16 bus, uint_16 slot, uint_16 func, uint_16 offset) {
        uint_32 address;
        uint_32 lbus = (uint_32)bus;
        uint_32 lslot = (uint_32)slot;
        uint_32 lfunc = (uint_32)func;
        
        // Build PCI Configuration Space Address (Mechanism #1)
        address = (uint_32)((lbus << 16) | (lslot << 11) |
                  (lfunc << 8) | (offset & 0xFC) | ((uint_32)0x80000000));
                  
        outl(0xCF8, address);
        
        // Read 32-bit value from 0xCFC
        return inl(0xCFC);
    }

    // Renamed from WriteWord to WriteDWord to reflect 32-bit write
    void WriteDWord(uint_16 bus, uint_16 slot, uint_16 func, uint_16 offset, uint_32 value) {
        uint_32 address;
        uint_32 lbus = (uint_32)bus;
        uint_32 lslot = (uint_32)slot;
        uint_32 lfunc = (uint_32)func;
        
        address = (uint_32)((lbus << 16) | (lslot << 11) |
                  (lfunc << 8) | (offset & 0xFC) | ((uint_32)0x80000000));
                  
        outl(0xCF8, address);
        outl(0xCFC, value);
    }

    uint_16 GetVendorID(uint_16 bus, uint_16 slot, uint_16 func) {
        uint_32 dword = ReadDWord(bus, slot, func, 0);
        return (uint_16)(dword & 0xFFFF);
    }

    uint_16 GetDeviceID(uint_16 bus, uint_16 slot, uint_16 func) {
        uint_32 dword = ReadDWord(bus, slot, func, 0);
        return (uint_16)(dword >> 16);
    }
    
    uint_8 GetClassId(uint_16 bus, uint_16 slot, uint_16 func) {
        uint_32 dword = ReadDWord(bus, slot, func, 0x08);
        return (uint_8)(dword >> 24);
    }
    
    uint_8 GetSubclassId(uint_16 bus, uint_16 slot, uint_16 func) {
        uint_32 dword = ReadDWord(bus, slot, func, 0x08);
        return (uint_8)((dword >> 16) & 0xFF);
    }
    
    uint_8 GetProgIF(uint_16 bus, uint_16 slot, uint_16 func) {
        uint_32 dword = ReadDWord(bus, slot, func, 0x08);
        return (uint_8)((dword >> 8) & 0xFF);
    }

    uint_32 GetBAR(uint_16 bus, uint_16 slot, uint_16 func, uint_8 bar) {
        uint_32 offset = 0x10 + (bar * 4);
        return ReadDWord(bus, slot, func, offset);
    }

    PCIDeviceHeader GetDeviceHeader(uint_16 bus, uint_16 slot, uint_16 func) {
        PCIDeviceHeader header;
        
        uint_32 dword0 = ReadDWord(bus, slot, func, 0x00);
        header.VendorID = (uint_16)(dword0 & 0xFFFF);
        header.DeviceID = (uint_16)(dword0 >> 16);
        
        uint_32 dword1 = ReadDWord(bus, slot, func, 0x04);
        header.Command = (uint_16)(dword1 & 0xFFFF);
        header.Status = (uint_16)(dword1 >> 16);
        
        uint_32 dword2 = ReadDWord(bus, slot, func, 0x08);
        header.RevisionID = (uint_8)(dword2 & 0xFF);
        header.ProgIF = (uint_8)((dword2 >> 8) & 0xFF);
        header.Subclass = (uint_8)((dword2 >> 16) & 0xFF);
        header.Class = (uint_8)(dword2 >> 24);
        
        uint_32 dword3 = ReadDWord(bus, slot, func, 0x0C);
        header.CacheLineSize = (uint_8)(dword3 & 0xFF);
        header.LatencyTimer = (uint_8)((dword3 >> 8) & 0xFF);
        header.HeaderType = (uint_8)((dword3 >> 16) & 0xFF);
        header.BIST = (uint_8)(dword3 >> 24);
        
        return header;
    }
}