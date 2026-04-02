PageTableEntry equ 0x20000

SetUpIdentityPaging:
    ; Zero out the page table region first
    mov edi, PageTableEntry
    mov ecx, 1024 * 4 ; 4 pages of 4KB
    xor eax, eax
    rep stosd

    mov edi, PageTableEntry
    mov eax, PageTableEntry
    add eax, 0x1003              ; EAX = PDP + flags
    mov [edi], eax               ; PML4[0] -> PDP

    add edi, 0x1000              ; EDI = PDP
    add eax, 0x1000              ; EAX = PD + flags
    mov [edi], eax               ; PDP[0] -> PD

    add edi, 0x1000              ; EDI = PD
    add eax, 0x1000              ; EAX = PT + flags
    mov [edi], eax               ; PD[0] -> PT

    add edi, 0x1000              ; EDI = PT
    mov ebx, 0x00000003
    mov ecx, 512
    .SetEntry:
        mov dword [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop .SetEntry
    
    mov eax, PageTableEntry
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5

    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    ret
