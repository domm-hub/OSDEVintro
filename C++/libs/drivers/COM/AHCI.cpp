#include "AHCI.h"
#include "BasicRenderer.h"
#include "Heap.h"
#include "Memory.h"
#include "TextPrint.h"
#include "PCI.h"
#include "Paging.h"

extern Paging::PageTableManager* GlobalPageTableManager;

namespace AHCI {

    Driver* GlobalAHCIDriver = nullptr;

    Port::Port(HBA_PORT* port, uint_8 num) {
        hbaPort = port;
        portNumber = num;
        configured = false;
        buffer = (uint_8*)malloc(512); 
    }

    void Port::StartCMD() {
        while (hbaPort->cmd & HBA_PxCMD_CR);
        hbaPort->cmd |= HBA_PxCMD_FRE;
        hbaPort->cmd |= HBA_PxCMD_ST;
    }

    void Port::StopCMD() {
        hbaPort->cmd &= ~HBA_PxCMD_ST;
        hbaPort->cmd &= ~HBA_PxCMD_FRE;
        
        uint_32 timeout = 1000000;
        while(timeout--) {
            if (hbaPort->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR)) continue;
            break;
        }
    }

    void Port::Configure() {
        StopCMD();

        void* newBase = aligned_alloc(1024, 1024);
        if (!newBase) return;
        memset(newBase, 0, 1024);
        uint_64 physBase = (uint_64)newBase; 
        
        hbaPort->clb = (uint_32)physBase;
        hbaPort->clbu = (uint_32)(physBase >> 32);

        void* fisBase = aligned_alloc(256, 256);
        if (!fisBase) return;
        memset(fisBase, 0, 256);
        uint_64 physFisBase = (uint_64)fisBase;
        
        hbaPort->fb = (uint_32)physFisBase;
        hbaPort->fbu = (uint_32)(physFisBase >> 32);

        HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)newBase;
        for (int i = 0; i < 32; i++) {
            cmdHeader[i].prdtl = 1; 
            
            void* cmdTable = aligned_alloc(128, 256);
            if (!cmdTable) break;
            memset(cmdTable, 0, 256);
            uint_64 physCmdTable = (uint_64)cmdTable;
            
            cmdHeader[i].ctba = (uint_32)physCmdTable;
            cmdHeader[i].ctbau = (uint_32)(physCmdTable >> 32);
        }

        StartCMD();
        configured = true;
    }

    bool Port::Read(uint_64 sector, uint_32 sectorCount, void* buffer) {
        if (sectorCount == 0 || buffer == nullptr) return false;
        hbaPort->is = 0xFFFFFFFF;
        hbaPort->serr = 0xFFFFFFFF;
        int slot = 0; 
        uint_64 clb_phys = ((uint_64)hbaPort->clbu << 32) | hbaPort->clb;
        HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)clb_phys;
        if (!cmdHeader) return false;
        cmdHeader += slot;
        cmdHeader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint_32);
        cmdHeader->w = 0; 
        cmdHeader->prdtl = 1;
        uint_64 ctba_phys = ((uint_64)cmdHeader->ctbau << 32) | cmdHeader->ctba;
        HBA_CMD_TBL* cmdTable = (HBA_CMD_TBL*)ctba_phys;
        if (!cmdTable) return false;
        memset(cmdTable, 0, 256);
        cmdTable->prdt_entry[0].dba = (uint_32)(uint_64)buffer; 
        cmdTable->prdt_entry[0].dbau = (uint_32)((uint_64)buffer >> 32);
        cmdTable->prdt_entry[0].dbc = (sectorCount * 512) - 1;
        cmdTable->prdt_entry[0].i = 1;
        FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdTable->cfis);
        cmdfis->fis_type = FIS_TYPE_REG_H2D;
        cmdfis->c = 1; 
        cmdfis->command = 0x25; 
        cmdfis->lba0 = (uint_8)sector;
        cmdfis->lba1 = (uint_8)(sector >> 8);
        cmdfis->lba2 = (uint_8)(sector >> 16);
        cmdfis->device = 1<<6; 
        cmdfis->lba3 = (uint_8)(sector >> 24);
        cmdfis->lba4 = (uint_8)(sector >> 32);
        cmdfis->lba5 = (uint_8)(sector >> 40);
        cmdfis->countl = sectorCount & 0xFF;
        cmdfis->counth = (sectorCount >> 8) & 0xFF;
        uint_32 spin = 0;
        while ((hbaPort->tfd & (0x80 | 0x08)) && spin < 1000000) spin++;
        if (spin == 1000000) return false;
        __asm__ volatile("" ::: "memory");
        hbaPort->ci = 1 << slot; 
        while (1) {
            if ((hbaPort->ci & (1 << slot)) == 0) break;
            if (hbaPort->is & (1 << 30)) return false; 
        }
        return true;
    }

    Driver::Driver(uint_16 bus, uint_16 slot, uint_16 func) {
        portCount = 0;
        uint_32 commandReg = PCI::ReadDWord(bus, slot, func, 0x04);
        commandReg |= (1 << 2) | (1 << 1); 
        PCI::WriteDWord(bus, slot, func, 0x04, commandReg);

        uint_32 bar5 = PCI::GetBAR(bus, slot, func, 5);
        uint_64 abar_phys = bar5 & 0xFFFFFFF0;
        if ((bar5 & 0x06) == 0x04) {
            uint_32 upper = PCI::GetBAR(bus, slot, func, 6);
            abar_phys |= ((uint_64)upper << 32);
        }

        abar = (HBA_MEM*)abar_phys;
        abar->ghc |= (1 << 31);
        abar->ghc |= 1;
        uint_32 timeout = 1000000;
        while ((abar->ghc & 1) && timeout--) __asm__("pause");
        abar->ghc |= (1 << 31);

        uint_32 pi = abar->pi;
        for (int i = 0; i < 32; i++) {
            if (pi & (1 << i)) {
                uint_32 timeout = 10000; while(timeout--) __asm__("pause");
                uint_32 ssts = abar->ports[i].ssts;
                uint_8 det = ssts & 0x0F;
                if (det == 0x03) {
                    uint32_t sig = abar->ports[i].sig;
                    if (GlobalRenderer) {
                        GlobalRenderer->Print("AHCI Port "); GlobalRenderer->Print(IntegerToString(i));
                        GlobalRenderer->Print(" Sig: "); GlobalRenderer->Print(HexToString(sig));
                        GlobalRenderer->Print("\n");
                    }
                    void* portMem = malloc(sizeof(Port));
                    if (portMem) {
                        ports[portCount] = new (portMem) Port(&abar->ports[i], i);
                        ports[portCount]->Configure();
                        portCount++;
                    }
                }
            }
        }
    }

    void Init() {
        if (GlobalAHCIDriver != nullptr) return; 
        for (uint_16 bus = 0; bus < 256; bus++) {
            for (uint_16 slot = 0; slot < 32; slot++) {
                for (uint_16 func = 0; func < 8; func++) {
                    uint_16 vendorID = PCI::GetVendorID(bus, slot, func);
                    if (vendorID == 0xFFFF) continue;
                    uint_8 classID = PCI::GetClassId(bus, slot, func);
                    uint_8 subclassID = PCI::GetSubclassId(bus, slot, func);
                    if (classID == 0x01 && subclassID == 0x06) {
                        GlobalAHCIDriver = (Driver*)malloc(sizeof(Driver));
                        if (GlobalAHCIDriver) new (GlobalAHCIDriver) Driver(bus, slot, func);
                        return;
                    }
                }
            }
        }
    }
}
