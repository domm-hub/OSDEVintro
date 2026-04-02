
DetectCPUID:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21

    push eax
    popfd

    pushfd
    pop eax

    push ecx
    popfd


    xor eax, ecx
    jz NoCPUID
    ret

DetectLongMode:
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz NoLongMode ; If Long Mode is not supported, halt the CPU.
    ret

NoLongMode:
    hlt ; No Long Mode support, halt the CPU

NoCPUID:
    hlt ; No CPUID support, halt the CPU