#pragma once
#include "TypeDefs.h"
#include "str.h" 
#include "Vector.h" 

namespace FAT32 {

    // Must be packed so the compiler doesn't add padding bytes
    #pragma pack(push, 1)
    struct BPB {
        uint8_t jump[3];
        char oem[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t num_fats;
        uint16_t root_dir_entries; // 0 for FAT32
        uint16_t total_sectors_short; // 0 for FAT32
        uint8_t media_descriptor;
        uint16_t sectors_per_fat; // 0 for FAT32
        uint16_t sectors_per_track;
        uint16_t heads;
        uint32_t hidden_sectors;
        uint32_t total_sectors_long;
        
        // FAT32 Extended Boot Record fields
        uint32_t sectors_per_fat_32;
        uint16_t flags;
        uint16_t version;
        uint32_t root_cluster;
        uint16_t fs_info_cluster;
        uint16_t backup_boot_sector;
        uint8_t reserved[12];
        uint8_t drive_number;
        uint8_t nt_flags;
        uint8_t signature;
        uint32_t volume_id;
        char volume_label[11];
        char system_id[8];
    };

    struct RawDirectoryEntry {
        char name[11];
        uint8_t attributes;
        uint8_t reserved;
        uint8_t creation_time_tenths;
        uint16_t creation_time;
        uint16_t creation_date;
        uint16_t last_access_date;
        uint16_t first_cluster_high;
        uint16_t last_modification_time;
        uint16_t last_modification_date;
        uint16_t first_cluster_low;
        uint32_t file_size;
    };
    #pragma pack(pop)

    struct File {
        String Name;
        uint32_t Size;
        uint32_t FirstCluster;
        bool IsDirectory;
    };

    class Driver {
        private:
            BPB bpb;
            uint32_t first_data_sector;
            uint32_t partitionOffset;
            uint32_t root_cluster;     
            
            uint32_t GetNextCluster(uint32_t cluster);

        public:
            bool (*RawDiskRead)(uint64_t, uint32_t, void*);
            Driver(bool (*readFunc)(uint64_t, uint32_t, void*), uint32_t partitionOffset);
            
            uint32_t ClusterToLBA(uint32_t cluster);
            Vector<File> ListDirectory(uint32_t cluster);
            Vector<File> ListRootDirectory();
            uint8_t* ReadFile(File file);
            uint32_t GetRootCluster() const;
    };


}