[extern _idt]
idtDescriptor:
    dw 4095
    dq _idt

[extern isr1_handler]
isr1:
    call isr1_handler
    iretq
    GLOBAL isr1


LoadIDT:
    lidt [idtDescriptor]
    sti
    ret
    GLOBAL LoadIDT