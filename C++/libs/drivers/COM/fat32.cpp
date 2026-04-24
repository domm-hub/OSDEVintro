#include "fat32.h"
#include "Heap.h" // for malloc/free
#include "BasicRenderer.h"
#include "Memory.h"
#include "TextPrint.h"

namespace FAT32 {

    Driver::Driver(bool (*readFunc)(uint64_t, uint32_t, void*), uint32_t partitionOffset) {
        this->RawDiskRead = readFunc;
        this->partitionOffset = partitionOffset; 

        uint8_t* bootSectorBuffer = (uint8_t*)malloc(512);
        if (bootSectorBuffer) {
            memset(bootSectorBuffer, 0, 512);
            if (!RawDiskRead(partitionOffset, 1, bootSectorBuffer)) {
                if (GlobalRenderer) GlobalRenderer->Print("FAT32 Error: Failed to read Boot Sector.\n");
            } else {
                memcpy(&bpb, bootSectorBuffer, sizeof(BPB));
                
                if (GlobalRenderer) {
                    GlobalRenderer->Print("BPB RAW 0-15: ");
                    for(int i=0; i<16; i++) {
                        GlobalRenderer->Print(HexToString(bootSectorBuffer[i]));
                        GlobalRenderer->Print(" ");
                    }
                    GlobalRenderer->Print("\n  OEM: ");
                    char oem[9]; memcpy(oem, bpb.oem, 8); oem[8] = 0;
                    GlobalRenderer->Print(oem);
                    GlobalRenderer->Print("\n  BytesPerSec: "); GlobalRenderer->Print(IntegerToString(bpb.bytes_per_sector));
                    GlobalRenderer->Print(" SecPerClus: "); GlobalRenderer->Print(IntegerToString(bpb.sectors_per_cluster));
                    GlobalRenderer->Print("\n  ResSec: "); GlobalRenderer->Print(IntegerToString(bpb.reserved_sectors));
                    GlobalRenderer->Print(" RootClus: "); GlobalRenderer->Print(IntegerToString(bpb.root_cluster));
                }
            }
            free(bootSectorBuffer);
        }

        uint32_t fat_size = (bpb.sectors_per_fat == 0) ? bpb.sectors_per_fat_32 : bpb.sectors_per_fat;
        first_data_sector = partitionOffset + bpb.reserved_sectors + (bpb.num_fats * fat_size);
        root_cluster = bpb.root_cluster;
        if (root_cluster < 2) root_cluster = 2; 

        if (GlobalRenderer) {
            GlobalRenderer->Print("\n  Data LBA: "); GlobalRenderer->Print(IntegerToString(first_data_sector));
            GlobalRenderer->Print("\n");
        }
    }

    uint32_t Driver::ClusterToLBA(uint32_t cluster) {
        return first_data_sector + ((cluster - 2) * bpb.sectors_per_cluster);
    }

    Vector<File> Driver::ListDirectory(uint32_t cluster) {
        if (cluster == 0) cluster = root_cluster;
        Vector<File> files;
        uint32_t currentCluster = cluster;
        uint32_t cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        
        if (cluster_size == 0) {
            if (GlobalRenderer) GlobalRenderer->Print("LS Error: Cluster size is 0!\n");
            return files;
        }

        RawDirectoryEntry* buffer = (RawDirectoryEntry*)malloc(cluster_size);
        if (!buffer) return files;

        while (currentCluster >= 2 && currentCluster < 0x0FFFFFF8) {
            uint32_t lba = ClusterToLBA(currentCluster);
            
            if (GlobalRenderer) {
                GlobalRenderer->Print("LS Reading LBA: ");
                GlobalRenderer->Print(IntegerToString(lba));
            }

            memset(buffer, 0, cluster_size);
            if (!RawDiskRead(lba, bpb.sectors_per_cluster, buffer)) {
                if (GlobalRenderer) GlobalRenderer->Print(" - FAILED\n");
                break;
            }

            if (GlobalRenderer) {
                GlobalRenderer->Print(" Peek: ");
                uint8_t* p = (uint8_t*)buffer;
                for(int x=0; x<8; x++) {
                    GlobalRenderer->Print(HexToString(p[x]));
                    GlobalRenderer->Print(" ");
                }
                GlobalRenderer->Print("\n");
            }

            for (int i = 0; i < (int)(cluster_size / sizeof(RawDirectoryEntry)); i++) {
                if (buffer[i].name[0] == 0x00) goto end_list; 
                if ((uint8_t)buffer[i].name[0] == 0xE5) continue; 
                if (buffer[i].attributes == 0x0F) continue; 
                if (buffer[i].attributes & 0x08) continue; 

                File f;
                char cleanName[13];
                int charIdx = 0;
                for(int j = 0; j < 8; j++) if(buffer[i].name[j] != ' ' && buffer[i].name[j] != 0) cleanName[charIdx++] = buffer[i].name[j];
                if (buffer[i].name[8] != ' ' && buffer[i].name[8] != 0) {
                    cleanName[charIdx++] = '.';
                    for(int j = 8; j < 11; j++) if(buffer[i].name[j] != ' ' && buffer[i].name[j] != 0) cleanName[charIdx++] = buffer[i].name[j];
                }
                cleanName[charIdx] = '\0';
                
                f.Name = String(cleanName);
                f.Size = buffer[i].file_size;
                f.FirstCluster = ((uint32_t)buffer[i].first_cluster_high << 16) | buffer[i].first_cluster_low;
                f.IsDirectory = (buffer[i].attributes & 0x10); 
                files.append(f);
            }
            currentCluster = GetNextCluster(currentCluster);
            if (currentCluster == 0) break;
        }

    end_list:
        free(buffer);
        return files;
    }

    Vector<File> Driver::ListRootDirectory() {
        return ListDirectory(root_cluster);
    }

    uint8_t* Driver::ReadFile(File file) {
        if (file.Size == 0) return nullptr;
        uint32_t cluster = file.FirstCluster;
        if (cluster == 0) cluster = root_cluster;
        uint32_t cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        if (cluster_size == 0) return nullptr;
        uint32_t totalClusters = (file.Size + cluster_size - 1) / cluster_size;
        uint32_t bufferSize = totalClusters * cluster_size;
        uint8_t* fileBuffer = (uint8_t*)aligned_alloc(4096, bufferSize + 4096); 
        if (!fileBuffer) return nullptr;
        uint32_t currentCluster = cluster;
        uint32_t clusterIdx = 0;
        while (currentCluster >= 2 && currentCluster < 0x0FFFFFF8 && clusterIdx < totalClusters) {
            uint32_t lba = ClusterToLBA(currentCluster);
            if (!RawDiskRead(lba, bpb.sectors_per_cluster, fileBuffer + (clusterIdx * cluster_size))) break;
            clusterIdx++;
            currentCluster = GetNextCluster(currentCluster);
            if (currentCluster == 0) break;
        }
        fileBuffer[file.Size] = '\0';
        return fileBuffer;
    }

    uint32_t Driver::GetNextCluster(uint32_t cluster) {
        uint32_t fat_start = partitionOffset + bpb.reserved_sectors;
        uint32_t fat_offset = cluster * 4;
        uint32_t fat_sector = fat_start + (fat_offset / bpb.bytes_per_sector);
        uint32_t ent_offset = fat_offset % bpb.bytes_per_sector;
        uint8_t* buffer = (uint8_t*)malloc(bpb.bytes_per_sector);
        if (!buffer) return 0x0FFFFFF7; 
        if (!RawDiskRead(fat_sector, 1, buffer)) { free(buffer); return 0x0FFFFFF7; }
        uint32_t nextCluster = (*(uint32_t*)&buffer[ent_offset]) & 0x0FFFFFFF;
        free(buffer);
        return nextCluster;
    }

    uint32_t Driver::GetRootCluster() const {
        return root_cluster;
    }
}
