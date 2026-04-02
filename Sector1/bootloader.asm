[org 0x7C00]

mov [BOOT_DISK], dl

mov bp, 0x7C00
mov sp, 0x7C00 

call ReadDisk

jmp PROGRAM_SPACE

%include "Sector1/print.asm"
%include "Sector1/DiskRead.asm"

times 510-($-$$) db 0
dw 0xaa55
