#pragma once
#include "TypeDefs.h"
#include "setjmp.h"

#define SYS_EXIT   0
#define SYS_PRINT  1
#define SYS_INPUT  2
#define SYS_SLEEP  3
#define SYS_YIELD  4
#define SYS_SBRK   5
#define SYS_UPTIME 6

void InitializeSyscalls();

extern "C" void SyscallHandlerEntry();

// C++ handler called from assembly
extern "C" uint_64 SyscallDispatcher(uint_64 syscall_number, uint_64 arg1, uint_64 arg2, uint_64 arg3);

// Buffer to store kernel state before jumping to userspace
extern jmp_buf shell_context;
