#include "fat32.h"
#include "Heap.h" // for malloc/free
#include "BasicRenderer.h"
#include "Memory.h"

namespace FAT32 {

    Driver::Driver(bool (*readFunc)(uint64_t, uint32_t, void*), uint32_t partitionOffset) {
        this->RawDiskRead = readFunc;
        this->partitionOffset = partitionOffset; 

        // Read Boot Sector (LBA offset) into a safe 512-byte buffer
        uint8_t* bootSectorBuffer = (uint8_t*)malloc(512);
        if (bootSectorBuffer) {
            // Read from the partition start, not physical disk sector 0
            if (!RawDiskRead(partitionOffset, 1, bootSectorBuffer)) {
                if (GlobalRenderer) GlobalRenderer->Print("FAT32 Error: Failed to read Boot Sector.\n");
            } else {
                memcpy(&bpb, bootSectorBuffer, sizeof(BPB));
            }
            free(bootSectorBuffer);
        }

        // Handle FAT32 Extended Boot Record 32-bit FAT size
        uint32_t fat_size = (bpb.sectors_per_fat == 0) ? bpb.sectors_per_fat_32 : bpb.sectors_per_fat;
        
        // The first data sector must be relative to the partition start
        first_data_sector = partitionOffset + bpb.reserved_sectors + (bpb.num_fats * fat_size);

        // Store the root cluster for easy access later (usually cluster 2)
        root_cluster = bpb.root_cluster;
    }

    uint32_t Driver::ClusterToLBA(uint32_t cluster) {
        return first_data_sector + ((cluster - 2) * bpb.sectors_per_cluster);
    }

    Vector<File> Driver::ListDirectory(uint32_t cluster) {
        Vector<File> files;
        uint32_t currentCluster = cluster;
        uint32_t cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        RawDirectoryEntry* buffer = (RawDirectoryEntry*)malloc(cluster_size);

        if (!buffer) return files;

        while (currentCluster >= 2 && currentCluster < 0x0FFFFFF8) {
            uint32_t lba = ClusterToLBA(currentCluster);
            if (!RawDiskRead(lba, bpb.sectors_per_cluster, buffer)) {
                break;
            }

            for (int i = 0; i < cluster_size / sizeof(RawDirectoryEntry); i++) {
                if (buffer[i].name[0] == 0x00) {
                    free(buffer);
                    return files; 
                }
                if ((uint8_t)buffer[i].name[0] == 0xE5) continue; 
                if (buffer[i].attributes == 0x0F) continue; 

                File f;
                char cleanName[13];
                int charIdx = 0;
                
                for(int j = 0; j < 8; j++) {
                    if(buffer[i].name[j] != ' ') cleanName[charIdx++] = buffer[i].name[j];
                }

                if (buffer[i].name[8] != ' ') {
                    cleanName[charIdx++] = '.';
                    for(int j = 8; j < 11; j++) {
                        if(buffer[i].name[j] != ' ') cleanName[charIdx++] = buffer[i].name[j];
                    }
                }
                cleanName[charIdx] = '\0';
                
                f.Name = String(cleanName);
                f.Size = buffer[i].file_size;
                f.FirstCluster = ((uint32_t)buffer[i].first_cluster_high << 16) | buffer[i].first_cluster_low;
                f.IsDirectory = (buffer[i].attributes & 0x10); 
                
                files.append(f);
            }

            currentCluster = GetNextCluster(currentCluster);
        }

        free(buffer);
        return files;
    }

    Vector<File> Driver::ListRootDirectory() {
        return ListDirectory(root_cluster);
    }

    uint8_t* Driver::ReadFile(File file) {
        if (file.Size == 0) return nullptr;

        uint32_t cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        // Total bytes to read, aligned to cluster size.
        uint32_t totalClusters = (file.Size + cluster_size - 1) / cluster_size;
        uint32_t bufferSize = totalClusters * cluster_size;

        // Allocate a little extra just in case (e.g. for a null terminator if the user wants to print it)
        // Use aligned_alloc to ensure the physical buffer starts at a page boundary (4096 bytes)
        uint8_t* fileBuffer = (uint8_t*)aligned_alloc(4096, bufferSize + 4096); 
        if (!fileBuffer) return nullptr;

        uint32_t currentCluster = file.FirstCluster;
        uint32_t clusterIdx = 0;

        while (currentCluster >= 2 && currentCluster < 0x0FFFFFF8 && clusterIdx < totalClusters) {
            uint32_t lba = ClusterToLBA(currentCluster);
            if (!RawDiskRead(lba, bpb.sectors_per_cluster, fileBuffer + (clusterIdx * cluster_size))) {
                break;
            }
            clusterIdx++;
            currentCluster = GetNextCluster(currentCluster);
        }

        // Add a null terminator at the end of the file data for convenience
        fileBuffer[file.Size] = '\0';

        return fileBuffer;
    }

    uint32_t Driver::GetNextCluster(uint32_t cluster) {
        uint32_t fat_start = partitionOffset + bpb.reserved_sectors;
        uint32_t fat_offset = cluster * 4;
        uint32_t fat_sector = fat_start + (fat_offset / bpb.bytes_per_sector);
        uint32_t ent_offset = fat_offset % bpb.bytes_per_sector;

        uint8_t* buffer = (uint8_t*)malloc(bpb.bytes_per_sector);
        if (!buffer) return 0x0FFFFFF7; // Bad cluster marker

        if (!RawDiskRead(fat_sector, 1, buffer)) {
            free(buffer);
            return 0x0FFFFFF7;
        }

        uint32_t nextCluster = (*(uint32_t*)&buffer[ent_offset]) & 0x0FFFFFFF;
        free(buffer);
        return nextCluster;
    }
}