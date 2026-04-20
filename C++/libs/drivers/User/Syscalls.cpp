#include "Syscalls.h"
#include "IO.h"
#include "GDT.h"
#include "BasicRenderer.h"
#include "setjmp.h"
#include "Keyboard.h"
#include "PIT.h"

jmp_buf shell_context;
extern "C" uint_64 KernelStackPtr = 0;

#define SYS_EXIT   0
#define SYS_PRINT  1
#define SYS_INPUT  2
#define SYS_SLEEP  3


void InitializeSyscalls() {
    // 1. Disable SMEP (Bit 20) and SMAP (Bit 21) in CR4
    // This allows the kernel to read user-mode memory (like strings) safely.
    uint_64 cr4;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 &= ~(1ULL << 20); // Clear SMEP
    cr4 &= ~(1ULL << 21); // Clear SMAP
    __asm__ volatile ("mov %0, %%cr4" : : "r"(cr4));

    // 2. Enable SCE (System Call Enable) in EFER
    uint_64 efer = rdmsr(0xC0000080);
    wrmsr(0xC0000080, efer | 1);

    // 3. Setup STAR MSR
    // Base selector for User mode is 0x18. 
    // SYSRET will use CS=0x28 (base+16) and SS=0x20 (base+8)
    uint_64 star = ((uint_64)0x08 << 32) | ((uint_64)0x1B << 48); 
    wrmsr(0xC0000081, star);

    // 4. LSTAR (Entry Point)
    wrmsr(0xC0000082, (uint_64)SyscallHandlerEntry);

    // 5. SFMASK (Interrupts are disabled during syscall)
    wrmsr(0xC0000084, 0x200); 
}


extern "C" uint_64 SyscallDispatcher(uint_64 syscall_number, uint_64 arg1, uint_64 arg2, uint_64 arg3) {
    switch (syscall_number) {
        case SYS_EXIT:
            returnCode = (uint_8)arg1;
            if (returnCode != 0){
                GlobalRenderer->Print("Program exited with non-zero return code.");
            }
            longjmp(shell_context, 1);

            return 0; 
        case SYS_PRINT: // PRINT
            if (GlobalRenderer) {
                GlobalRenderer->Print((const char*)arg1);
            }
            return 0;
        case SYS_INPUT:
            return prompt((const char*)arg1);
        case SYS_SLEEP:
            return PIT::Sleep(arg1);

        default:
            return (uint_64)-1;
    }
}
