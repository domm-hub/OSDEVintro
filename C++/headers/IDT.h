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
    uint_32 zero;
};

extern IDT64 _idt[256];
extern uint_64 isr1;
extern uint_64 isr13;
extern "C" void LoadIDT();

extern void (*MainKeyboardHandler)(uint_8 scanCode, uint_8 chr);

void MakeIDTEntry(uint_64 handler, uint_16 index, uint_8 selector, uint_8 types_attr);
void InitializeIDT();

extern "C" void GPF_Handler(const char* message, uint_64 errorCode);
extern "C" void isr1_handler();