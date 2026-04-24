#pragma once
#include "TypeDefs.h"

// Ensure the compiler doesn't pad these structures
#pragma pack(push, 1)

struct PartitionTableEntry {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sector_cylinder[2]; // Simplified to avoid bitfield issues
    uint8_t partition_id;             // 0x0B or 0x0C indicates FAT32
    uint8_t end_head;
    uint8_t end_sector_cylinder[2];
    uint32_t lba_start;               // THIS is the partitionOffset we need
    uint32_t total_sectors;
};

struct MasterBootRecord {
    uint8_t bootloader[446];
    PartitionTableEntry primaryPartition[4];
    uint16_t boot_signature;  // Should be 0xAA55
};

#pragma pack(pop)
