gdt_nulldesc:
    dd 0
    dd 0

gdt_code_desc:
    dw 0xFFFF 
    dw 0x0000 
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data_desc:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

GDT_descriptor:
    gdt_size:
        dw gdt_end - gdt_nulldesc - 1
        dd gdt_nulldesc

codeseg equ gdt_code_desc - gdt_nulldesc
dataseg equ gdt_data_desc - gdt_nulldesc

[bits 32]
editgdt:
    mov [gdt_code_desc + 6], byte 10101111b
    ret
[bits 16]
    