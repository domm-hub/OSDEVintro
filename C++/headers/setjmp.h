#pragma once
#include "TypeDefs.h"

// A buffer to hold: rbx, rbp, r12, r13, r14, r15, rsp, rip
typedef uint_64 jmp_buf[8]; 

extern "C" int setjmp(jmp_buf env);
extern "C" void longjmp(jmp_buf env, int val);