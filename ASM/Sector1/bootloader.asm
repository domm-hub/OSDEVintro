[bits 16]
[org 0x7C00]

; Initialize segment registers
xor ax, ax
mov ds, ax
mov es, ax

mov [BOOT_DISK], dl

mov bp, 0x7C00
mov sp, 0x7C00 

call ReadDisk

jmp PROGRAM_SPACE

%include "ASM/Sector1/print.asm"
%include "ASM/Sector1/DiskRead.asm"

times 510-($-$$) db 0
dw 0xaa55
