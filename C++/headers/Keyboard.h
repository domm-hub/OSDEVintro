#pragma once
#include "TypeDefs.h"
#include "TextPrint.h"


extern "C" void GlobalPutChar(char c, uint_32 color = 0xFFFFFFFF);

__attribute__((no_caller_saved_registers))
void KeyboardHandler(uint_8 scanCode, uint_8 chr);