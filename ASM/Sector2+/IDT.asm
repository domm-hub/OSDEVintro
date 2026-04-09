[extern _idt]

idtDescriptor:
    dw 4095
    dq _idt

%macro PUSHALL 0
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro POPALL 0
    pop rax
    pop rcx
    pop rdx
    pop r8
    pop r9
    pop r10
    pop r11
%endmacro

[extern isr1_handler]
isr1:
    PUSHALL
    call isr1_handler
    POPALL
    iretq
    GLOBAL isr1

    ; We will write this in C++
global isr13


gpf_msg db "FATAL: GENERAL PROTECTION FAULT", 0


extern GPF_Handler
global isr13


isr13:
    ; 1. Save all registers so we don't lose data
    push rax
    push rbx
    push rcx
    push rdx
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

    ; 2. Calculate where the CPU saved the info
    ; We pushed 15 registers (15 * 8 = 120 bytes)
    ; So the Error Code is at [rsp + 120]
    ; And the RIP is at [rsp + 128]

    mov rdi, gpf_msg       ; 1st Arg: String
    mov rsi, [rsp + 120]   ; 2nd Arg: Error Code
    mov rdx, [rsp + 128]   ; 3rd Arg: RIP (Instruction Pointer)

    call GPF_Handler

LoadIDT:
    lidt [idtDescriptor]
    sti
    ret
    GLOBAL LoadIDT