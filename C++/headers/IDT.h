#pragma once

#include "TypeDefs.h"
#include "IO.h"
#include "TextPrint.h"
#include "KBSCodesS1.h"

struct IDT64 {
    uint_16 offset_low;
    uint_16 selector;
    uint_8 ist;
    uint_8 types_attr;
    uint_16 offset_mid;
    uint_32 offset_high;
    uint_32 reserved; // Reserved / must be zero
} __attribute__((packed));

extern IDT64 _idt[256];
extern "C" __attribute__((interrupt)) void isr1(void* frame);
extern "C" __attribute__((interrupt)) void isr13(void* frame, uint_64 errorCode);
extern "C" void LoadIDT();

__attribute__((no_caller_saved_registers))
extern void (*MainKeyboardHandler)(uint_8 scanCode, uint_8 chr);

void MakeIDTEntry(uint_64 handler, uint_16 index, uint_16 selector, uint_8 types_attr);
void InitializeIDT();

__attribute__((no_caller_saved_registers))
extern "C" void GPF_Handler(const char* message, uint_64 errorCode);