[bits 16]
jmp EnterProtectedMode


%include "Sector1/print.asm"
%include "Sector2+/gdt.asm"

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

%include "Sector2+/CPUID.asm"
%include "Sector2+/paging.asm"

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
Start64Bit:
    mov edi, 0xb8000
    mov rax, 0x1f201f201f201f20
    mov ecx, 500
    rep stosq
    call _start
    jmp $