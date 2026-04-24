#include "Syscalls.h"
#include "IO.h"
#include "GDT.h"
#include "BasicRenderer.h"
#include "setjmp.h"
#include "Keyboard.h"
#include "PIT.h"
#include "Multitask.h"
#include "Heap.h"
#include "Memory.h"
#include "PageFrameAllocator.h"

jmp_buf shell_context;
extern "C" uint_64 KernelStackPtr = 0;

void* sys_sbrk(int_64 increment) {
    Process* proc = Scheduler::GetCurrentProcess();
    void* old_brk = (void*)proc->CurrentBreak;
    
    if (increment == 0) return old_brk;

    uint_64 new_brk = proc->CurrentBreak + increment;

    if (increment > 0) {
        uint_64 start = (proc->CurrentBreak + 4095) & ~0xFFFULL;
        uint_64 end = (new_brk + 4095) & ~0xFFFULL;
        
        Paging::PageTableManager manager((Paging::PageTable*)proc->cr3Value);

        for (uint_64 addr = start; addr < end; addr += 4096) {
            void* phys = GlobalAllocator.RequestPage();
            memset(phys, 0, 4096);
            manager.MapMemory((void*)addr, phys, true);
        }
    }

    proc->CurrentBreak = new_brk;
    return old_brk;
}

void InitializeSyscalls() {
    uint_64 cr4;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 &= ~(1ULL << 20); 
    cr4 &= ~(1ULL << 21); 
    __asm__ volatile ("mov %0, %%cr4" : : "r"(cr4));

    uint_64 efer = rdmsr(0xC0000080);
    wrmsr(0xC0000080, efer | 1);

    uint_64 star = ((uint_64)0x08 << 32) | ((uint_64)0x1B << 48); 
    wrmsr(0xC0000081, star);

    wrmsr(0xC0000082, (uint_64)SyscallHandlerEntry);
    wrmsr(0xC0000084, 0x200); 
}


extern "C" uint64_t SyscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (syscall_number) {
        case SYS_EXIT: {
            longjmp(shell_context, 1);
            return 0; 
        }

        case SYS_PRINT: {
            if (GlobalRenderer) {
                GlobalRenderer->Print((const char*)arg1);
            }
            return 0;
        }

        case SYS_INPUT: {
            static String kernelInputBuffer;
            kernelInputBuffer = prompt((const char*)arg1);
            return (uint64_t)kernelInputBuffer.c_str(); 
        }
        
        case SYS_SLEEP: {
            PIT::Sleep(arg1);
            return 0;
        }

        case SYS_YIELD: {
            Scheduler::Yield();
            return 0;
        }
        case SYS_UPTIME: {
            return PIT::TicksSinceBoot;
        }

        case SYS_SBRK: {
            return (uint64_t)sys_sbrk((int_64)arg1);
        }

        default:
            return (uint64_t)-1;
    }
}
