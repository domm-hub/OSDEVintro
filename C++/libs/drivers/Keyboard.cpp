#include "TypeDefs.h"
#include "KBSCodesS1.h"
#include "../../headers/BasicRenderer.h"

#include "Keyboard.h"


bool LShift = false;
bool RShift = false;

__attribute__((no_caller_saved_registers))
void KeyboardHandler(uint_8 scanCode, uint_8 chr){
    GlobalRenderer->PutChar(chr);
}
