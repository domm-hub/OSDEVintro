.section .entry
.global _start
.extern kernel_main
.extern kernel_stack

_start:
    cli
    # UEFI passes BootInfo* in RCX
    mov %rcx, %rdi
    
    # Switch to kernel stack
    # RSP points to the TOP of the buffer
    movabs $kernel_stack, %rax
    movabs $65536, %rbx
    add %rbx, %rax
    sub $16, %rax
    mov %rax, %rsp
    
    # Align stack to 16 bytes for ABI
    and $-16, %rsp
    
    # Use an indirect call to avoid PLT32 relocation issues with some linkers
    movabs $kernel_main, %rax
    call *%rax

.loop:
    hlt
    jmp .loop
