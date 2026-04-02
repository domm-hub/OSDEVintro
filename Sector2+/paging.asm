PageTableEntry equ 0x1000

SetUpIdentityPaging:
    ; Zero out the page table region first
    mov edi, PageTableEntry
    mov ecx, 1024 * 4 ; 4 pages of 4KB
    xor eax, eax
    rep stosd

    mov edi, PageTableEntry
    mov dword [edi], 0x2003      ; PML4E (0x1000) -> PDP (0x2000)
    mov dword [edi + 0x1000], 0x3003 ; PDPE (0x2000) -> PD (0x3000)
    mov dword [edi + 0x2000], 0x4003 ; PDE (0x3000) -> PT (0x4000)
    add edi, 0x3000              ; EDI now points to PT (0x4000)

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
