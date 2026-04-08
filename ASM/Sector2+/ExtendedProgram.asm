[bits 16]
jmp EnterProtectedMode


%include "ASM/Sector1/print.asm"
%include "ASM/Sector2+/gdt.asm"

EnterProtectedMode:
    call EnableA20
    cli
    lgdt [GDT_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax 
    jmp codeseg:start_protected_mode
    

EnableA20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret


[bits 32]

%include "ASM/Sector2+/CPUID.asm"
%include "ASM/Sector2+/paging.asm"

start_protected_mode:
    mov ax, dataseg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    
    call DetectCPUID
    call DetectLongMode
    call editgdt
    call SetUpIdentityPaging

    jmp codeseg:Start64Bit

[bits 64]
[extern _start]

%include "ASM/Sector2+/IDT.asm"
Start64Bit:
    mov edi, 0xb8000
    mov rax, 0x1f201f201f201f20
    mov ecx, 500
    rep stosq
    call ActivateSSE
    call _start
    jmp $

ActivateSSE:
    mov rax, cr0
    and ax, 0b1111111111111011 
    or  ax, 0b0000000000100010 
    mov cr0, rax

    mov rax, cr4
    or  ax, 0b0000011000000000 
    mov cr4, rax

    ret


times 18432 - ($ - EnterProtectedMode) db 0