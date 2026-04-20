#pragma once
#include "TypeDefs.h"

struct GDTDescriptor {
    uint_16 Limit0;
    uint_16 Base0;
    uint_8  Base1;
    uint_8  Access;
    uint_8  Limit1_Flags;
    uint_8  Base2;
} __attribute__((packed));

struct TSS {
    uint_32 reserved0;
    uint_64 rsp0;
    uint_64 rsp1;
    uint_64 rsp2;
    uint_64 reserved1;
    uint_64 ist1;
    uint_64 ist2;
    uint_64 ist3;
    uint_64 ist4;
    uint_64 ist5;
    uint_64 ist6;
    uint_64 ist7;
    uint_64 reserved2;
    uint_16 reserved3;
    uint_16 iopb_offset;
} __attribute__((packed));

struct TSSDescriptorHigh {
    uint_32 Base3;      // Bits 32-63 of Base
    uint_32 Reserved;   // Must be 0
} __attribute__((packed));

struct GDTContainer {
    GDTDescriptor Null;
    GDTDescriptor KernelCode;
    GDTDescriptor KernelData;
    GDTDescriptor UserCode32;
    GDTDescriptor UserData;
    GDTDescriptor UserCode;
    GDTDescriptor TSS_Low;
    TSSDescriptorHigh TSS_High;
} __attribute__((packed, aligned(0x1000)));
extern GDTContainer DefaultGDT;
extern TSS GlobalTSS;

struct GDTR {
    uint_16 Limit;
    uint_64 Offset;
} __attribute__((packed));

extern "C" void LoadGDT(GDTR* gdtr);
extern "C" void LoadTR(uint_16 selector);
extern "C" void LoadCR3(uint_64 pml4Address);

void InitializeGDT();
