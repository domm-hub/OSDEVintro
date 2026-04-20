#include "Multitask.h"

Vector<Process> Processes;
uint64_t ProcessNumberCurrent;

void yield(){
	asm("hlt"); // for now, until i implement
	
}

struct Registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss; // These are pushed by the CPU on interrupt
};

struct Process {
	char 		ProcessName[32];
	uint_16     ProcessId;
	
	uint_16     Priority;
	uint_32     TicksLeft;
	bool        Working;

	uint_64		cr3Value;
	uint_64 	ParentPid;
	Registers   Context;

	uint_64 	CpuTimeUsed;
	uint_64		WakeTick;
};