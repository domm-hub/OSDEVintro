.section .entry
.global _start
.extern kernel_main

_start:
    cli
    # UEFI passes BootInfo* in RCX
    mov %rcx, %rdi
    
    # Align stack to 16 bytes
    # UEFI stack is 16-byte aligned, we are called via 'call', so RSP ends in 8
    sub $8, %rsp
    
    # Use an indirect call to avoid PLT32 relocation issues with some linkers
    movabs $kernel_main, %rax
    call *%rax

.loop:
    hlt
    jmp .loop
