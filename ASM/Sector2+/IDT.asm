[bits 64]
[extern _idt]
[extern isr1_handler]

%macro PUSHALL 0
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro POPALL 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
%endmacro

section .data
align 16
idtDescriptor:
    dw 4095      ; Limit: (256 * 16) - 1
    dq _idt      ; Base: 64-bit address

section .text
GLOBAL isr1
isr1:
    PUSHALL
    
    ; System V ABI requires 16-byte stack alignment for calls
    ; The CPU pushes 5 qwords (rip, cs, rflags, rsp, ss) on interrupt
    ; We pushed 9 qwords. 5 + 9 = 14. 
    ; We need an extra 8 bytes to make it 16-byte aligned (14 + 2 = 16)
    sub rsp, 8 
    
    call isr1_handler
    
    add rsp, 8 ; Clean up alignment
    
    POPALL
    iretq

GLOBAL LoadIDT
LoadIDT:
    lidt [idtDescriptor]
    sti
    ret