#include "TypeDefs.h"
#include "IO.h"
#include "BasicRenderer.h"
#include "KBSCodesS1.h"
#include "IDT.h"
#include "PIT.h"

__attribute__((no_caller_saved_registers))
void (*MainKeyboardHandler)(uint_8 scanCode, uint_8 chr) = 0;

struct IDTR {
    uint_16 Limit;
    uint_64 Offset;
} __attribute__((packed));

__attribute__((aligned(16)))
IDT64 _idt[256];

extern "C" __attribute__((interrupt)) void isr0(void* frame);
extern "C" __attribute__((interrupt)) void isr1(void* frame);
extern "C" __attribute__((interrupt)) void isr13(void* frame, uint_64 errorCode);
extern "C" __attribute__((interrupt)) void isr14(void* frame, uint_64 errorCode);

extern "C" void LoadIDT() {
    IDTR idtr;
    idtr.Limit = (sizeof(IDT64) * 256) - 1;
    idtr.Offset = (uint_64)_idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

uint_16 GetCS() {
    uint_16 cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

extern "C" __attribute__((interrupt)) void isr0(void* frame) {
    PIT::Tick();
    outb(0x20, 0x20);
}

extern "C" __attribute__((interrupt)) void isr1(void* frame) {
    uint_8 scanCode = inb(0x60);
    if (MainKeyboardHandler != 0){
        MainKeyboardHandler(scanCode, KBSet1::ScanCodeLookupTable[scanCode]);
    }
    outb(0x20, 0x20);
}

extern "C" __attribute__((interrupt)) void isr13(void* frame, uint_64 errorCode) {
    GPF_Handler("GENERAL PROTECTION FAULT", errorCode);
}

extern "C" __attribute__((interrupt)) void isr14(void* frame, uint_64 errorCode) {
    uint_64 cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    if (GlobalRenderer != nullptr) {
        GlobalRenderer->Clear(0x440000); 
        GlobalRenderer->Print("PAGE FAULT\n");
        GlobalRenderer->Print("Fault Address: 0x");
        GlobalRenderer->Print(HexToString(cr2));
    }
    while(1);
}

bool MakeIDTEntry(uint_64 handler, uint_16 index, uint_16 selector, uint_8 types_attr){
    _idt[index].offset_low  =  (uint_16)((handler & 0xFFFF));
    _idt[index].selector    =  selector;
    _idt[index].ist         =  0;
    _idt[index].types_attr  =  types_attr;
    _idt[index].offset_mid  =  (uint_16)((handler >> 16) & 0xFFFF);
    _idt[index].offset_high =  (uint_32)((handler >> 32) & 0xFFFFFFFF);
    _idt[index].reserved    =  0;
    return true;
}

void DisableAPIC() {
    uint_32 lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1B));
    lo &= ~(1 << 11); 
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(0x1B));
}

extern "C" void SchedulerInterruptStub();
extern "C" void PIT_InterruptStub();
extern "C" void MouseInterruptStub();

void InitializeIDT(){
    DisableAPIC(); // Mandatory for UEFI/OVMF to stop rogue IRQs
    RemapPic();

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    uint16_t cs = 0x08; 

    MakeIDTEntry((uint_64)PIT_InterruptStub, 32, cs, 0x8e);
    MakeIDTEntry((uint_64)isr1, 33, cs, 0x8e);
    MakeIDTEntry((uint_64)MouseInterruptStub, 44, cs, 0x8e); // IRQ 12 = 32 + 12 = 44
    MakeIDTEntry((uint_64)SchedulerInterruptStub, 0x80, cs, 0x8e);
    MakeIDTEntry((uint_64)isr13, 13, cs, 0x8e);
    MakeIDTEntry((uint_64)isr14, 14, cs, 0x8e);

    LoadIDT();

    while (inb(0x64) & 1) inb(0x60);

    outb(0x21, 0xF8); // Unmask IRQ 0 (PIT), 1 (Keyboard), 2 (Cascade to Slave)
    outb(0xA1, 0xEF); // Unmask IRQ 12 (Mouse)
}

__attribute__((no_caller_saved_registers))
extern "C" void GPF_Handler(const char* message, uint_64 errorCode) {
    if (GlobalRenderer != nullptr) {
        GlobalRenderer->Clear(0x440000);
        GlobalRenderer->Print(message);
        GlobalRenderer->Print("\nError: 0x");
        GlobalRenderer->Print(HexToString(errorCode));
    }
    while(1);
}
