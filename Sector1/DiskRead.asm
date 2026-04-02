PROGRAM_SPACE equ 0x7e00

ReadDisk:
    ; Reset the disk system first
    xor ax, ax
    mov dl, [BOOT_DISK]
    int 0x13
    jc .DiskError

    ; Setup memory location
    xor ax, ax
    mov es, ax
    mov bx, PROGRAM_SPACE
    
    ; Setup Disk Parameters for Sector 2 (ExtendedProgram)
    mov ah, 0x02       ; BIOS read sectors
    mov al, 8          ; Attempt to read 8 sectors
    mov dl, [BOOT_DISK]
    mov ch, 0          ; Cylinder 0
    mov dh, 0          ; Head 0
    mov cl, 2          ; Sector 2

    int 0x13
    jc .DiskError      ; Jump if carry flag is set (Read failed)
    
    ; Check AL to see how many sectors were actually read
    cmp al, 8
    jne .DiskError     ; If we read fewer than 4 sectors, it's an error
    ret

.DiskError:
    mov si, msg_disk_err
    call print
    jmp $

BOOT_DISK:
    db 0

msg_disk_err:
    db "Disk read failed!", 0x0D, 0x0A, 0
