#pragma once
#include "TypeDefs.h"
#include "setjmp.h"

void InitializeSyscalls();

extern "C" void SyscallHandlerEntry();

// C++ handler called from assembly
extern "C" uint_64 SyscallDispatcher(uint_64 syscall_number, uint_64 arg1, uint_64 arg2, uint_64 arg3);

// Buffer to store kernel state before jumping to userspace
extern jmp_buf shell_context;
