#pragma once
#include "TypeDefs.h"
extern "C" uint_64 do_syscall(uint_64 num, uint_64 a1, uint_64 a2, uint_64 a3) {
    uint_64 result;
    asm volatile (
        "mov %1, %%rax;"  // Syscall number into RAX
        "mov %2, %%rdi;"  // arg1 into RDI
        "mov %3, %%rsi;"  // arg2 into RSI
        "mov %4, %%rdx;"  // arg3 into RDX
        "syscall;"        // Jump to kernel!
        "mov %%rax, %0;"  // AFTER syscall, grab RAX back into 'result'
        : "=r"(result)    // Output: result variable
        : "r"(num), "r"(a1), "r"(a2), "r"(a3)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return result;
}

void exit(unsigned char returncode) {
    do_syscall(0, (uint_64)returncode, 0, 0);
}


void println(const char* msg) {
    return do_syscall(1, msg, 0, 0);
}


void input(const char* prmpt) {
    return do_syscall(2, prmpt, 0, 0);
}
