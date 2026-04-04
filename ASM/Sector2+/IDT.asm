[extern _idt]

%macro PUSHALL 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POPALL 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro



idtDescriptor:
    dw 4095
    dq _idt

[extern isr1_handler]
isr1:
    PUSHALL
    call isr1_handler
    POPALL
    iretq
    GLOBAL isr1

LoadIDT:
    lidt [idtDescriptor]
    sti
    ret
    GLOBAL LoadIDT