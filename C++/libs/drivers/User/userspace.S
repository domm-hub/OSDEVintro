.section .text
.global JumpToUser

JumpToUser:
    /* rdi = user_rip, rsi = user_rsp */
    
    cli
    
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    pushq $0x23       /* SS */
    pushq %rsi        /* RSP */
    pushfq            /* RFLAGS */
    popq %rax
    orq $0x200, %rax  /* Enable interrupts in user mode */
    pushq %rax        /* RFLAGS */
    pushq $0x2B       /* CS */
    pushq %rdi        /* RIP */
    
    iretq

.extern KernelStackPtr
.global SyscallHandlerEntry
SyscallHandlerEntry:
    /* 1. Switch to Kernel Stack safely */
    /* We stash user RSP in R10 because syscall overwrites RCX/R11 */
    movq %rsp, %r10             
    movq KernelStackPtr, %rsp   
    
    /* 2. Push context onto Kernel Stack */
    pushq %r10 /* User RSP */
    pushq %rcx /* User RIP */
    pushq %r11 /* User RFLAGS */
    pushq %rbp
    movq %rsp, %rbp

    /* 3. Map arguments to System V ABI */
    /* User: RAX (num), RDI, RSI, RDX */
    /* C++:  RDI (num), RSI, RDX, RCX */
    
    movq %rdx, %rcx /* arg3 -> RCX */
    movq %rsi, %rdx /* arg2 -> RDX */
    movq %rdi, %rsi /* arg1 -> RSI */
    movq %rax, %rdi /* syscall_number -> RDI */
    
    call SyscallDispatcher

    /* 4. Restore context */
    popq %rbp
    popq %r11 
    popq %rcx 
    popq %rsp 
    
    sysretq
