#include "GDT.h"
#include "Memory.h" 

GDTContainer DefaultGDT;
TSS GlobalTSS;

void InitializeGDT() {
    memset(&DefaultGDT, 0, sizeof(GDTContainer));
    memset(&GlobalTSS, 0, sizeof(TSS));

    // 1. Null Descriptor (0x00)
    DefaultGDT.Null = {0, 0, 0, 0, 0, 0};

    // 2. Kernel Code Segment (0x08)
    DefaultGDT.KernelCode = {0, 0, 0, 0x9A, 0x20, 0};

    // 3. Kernel Data Segment (0x10)
    DefaultGDT.KernelData = {0, 0, 0, 0x92, 0x00, 0};

    // 4. User Code 32-bit (0x18) - Access 0xFB, Flags 0xCF
    DefaultGDT.UserCode32 = {
        0xFFFF,     // Limit 0-15
        0,          // Base 0-15
        0,          // Base 16-23
        0xFB,       // Access: Present, Ring 3, Code, Exec, Read, Accessed
        0xCF,       // Flags: Granularity, 32-bit (Required for SYSRET base)
        0           // Base 24-31
    };

    // 5. User Data Segment (0x20) - Access 0xF3
    DefaultGDT.UserData = {
        0xFFFF, 
        0, 
        0, 
        0xF3,       // Access: Present, Ring 3, Data, Read, Write, Accessed
        0xCF,       // Flags: Granularity, 32-bit
        0
    };

    // 6. User Code 64-bit (0x28) - Access 0xFB, Flags 0xAF
    DefaultGDT.UserCode = {
        0xFFFF, 
        0, 
        0, 
        0xFB,       // Access: Present, Ring 3, Code, Exec, Read, Accessed
        0xAF,       // Flags: Granularity, 64-bit (Long)
        0
    };

    // 7. TSS Descriptor (0x30)
    uint_64 tss_base = (uint_64)&GlobalTSS;
    uint_32 tss_limit = sizeof(TSS) - 1;

    DefaultGDT.TSS_Low.Limit0 = (uint_16)(tss_limit & 0xFFFF);
    DefaultGDT.TSS_Low.Base0 = (uint_16)(tss_base & 0xFFFF);
    DefaultGDT.TSS_Low.Base1 = (uint_8)((tss_base >> 16) & 0xFF);
    DefaultGDT.TSS_Low.Access = 0x89; 
    DefaultGDT.TSS_Low.Limit1_Flags = (uint_8)((tss_limit >> 16) & 0x0F);
    DefaultGDT.TSS_Low.Base2 = (uint_8)((tss_base >> 24) & 0xFF);
    DefaultGDT.TSS_High.Base3 = (uint_32)((tss_base >> 32) & 0xFFFFFFFF);
    DefaultGDT.TSS_High.Reserved = 0; 

    GDTR gdtr;
    gdtr.Limit = sizeof(GDTContainer) - 1;
    gdtr.Offset = (uint_64)&DefaultGDT;
    
    LoadGDT(&gdtr);
    LoadTR(0x30);
}
