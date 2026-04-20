#include "Multitask.h"

Vector<Process> Processes;
uint64_t ProcessNumberCurrent;
uint_64 TicksPassedChange = 0;

void ChangeProcess() {
    // 1. Update the current process
    Process& current = Processes[ProcessNumberCurrent];
    
    // Check if it's actually time to switch
    if (current.TicksLeft > 0) {
        current.TicksLeft--;
        return; // Still has time, keep vibing
    }

    // 2. SAVE: Get the current CR3 (and you'd save registers here too)
    asm volatile("mov %%cr3, %0" : "=r"(current.cr3Value));
    
    // 3. SELECT: Find the next process that is actually "Working"
    int next_index = (ProcessNumberCurrent + 1) % TOTAL_PROCESSES;
    
    // Simple loop to find a process that isn't sleeping
    while (!Processes[next_index].Working) {
        next_index = (next_index + 1) % TOTAL_PROCESSES;
    }

    ProcessNumberCurrent = next_index;
    Process& next = Processes[ProcessNumberCurrent];

    // 4. LOAD: Reset its ticks and swap the memory map
    next.TicksLeft = next.Priority; // Give it a fresh turn based on priority
    asm volatile("mov %0, %%cr3" :: "r"(next.cr3Value));
}


void InitializeKernelProcess() {
    Process& kernel = Processes;
    kernel.ProcessId = 0;
    kernel.Priority = 1; 
    kernel.Working = true; // The kernel is ALWAYS working
    kernel.ProcessName = "Kernel";
    
    // The kernel is already running, so we grab its current CR3
    asm volatile("mov %%cr3, %0" : "=r"(kernel.cr3Value));
    
    ProcessNumberCurrent = 0; // Tell the scheduler we are starting here
}

struct Registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss; // These are pushed by the CPU on interrupt
};

struct Process {
	char 		ProcessName[32];
	uint_16     ProcessId;
	
	uint_16     Priority = 2;
	uint_32     TicksLeft = 1;
	bool        Working = true;

	uint_64		cr3Value;
	uint_64 	ParentPid;
	Registers   Context;

	uint_64 	CpuTimeUsed;
};