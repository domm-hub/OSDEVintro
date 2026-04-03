PROGRAM_SPACE equ 0x8000

ReadDisk:
    mov [BOOT_DISK], dl
    
    ; 1. Reset Disk (BIOS might need a reset)
    mov ah, 0
    mov dl, [BOOT_DISK]
    int 0x13
    jc .DiskError

    ; 2. Load Track 0, Head 0 (Sectors 2-18 = 17 sectors)
    mov ax, 0
    mov es, ax
    mov bx, PROGRAM_SPACE
    
    mov di, 3          ; Retry 3 times
.Read1:
    mov ah, 0x02
    mov al, 17         ; 17 sectors
    mov ch, 0          ; Cylinder 0
    mov dh, 0          ; Head 0
    mov cl, 2          ; Start at Sector 2
    mov dl, [BOOT_DISK]
    int 0x13
    jnc .Done1
    dec di
    jnz .Read1
    jmp .DiskError
.Done1:

    ; 3. Update Buffer Pointer
    ; 17 sectors * 512 = 8704 bytes (0x2200)
    add bx, 0x2200

    ; 4. Load Track 0, Head 1 (Sectors 1-18 = 18 sectors)
    mov di, 3          ; Retry 3 times
.Read2:
    mov ah, 0x02
    mov al, 18         ; 18 sectors
    mov ch, 0          ; Cylinder 0
    mov dh, 1          ; Head 1
    mov cl, 1          ; Start at Sector 1
    mov dl, [BOOT_DISK]
    int 0x13
    jnc .Done2
    dec di
    jnz .Read2
    jmp .DiskError
.Done2:

    ; 5. Update Buffer Pointer
    ; 18 sectors * 512 = 9216 bytes (0x2400)
    add bx, 0x2400

    ; 6. Load Track 1, Head 0 (Sectors 1-18 = 18 sectors)
    mov di, 3          ; Retry 3 times
.Read3:
    mov ah, 0x02
    mov al, 18         ; 18 sectors
    mov ch, 1          ; Cylinder 1
    mov dh, 0          ; Head 0
    mov cl, 1          ; Start at Sector 1
    mov dl, [BOOT_DISK]
    int 0x13
    jnc .Done3
    dec di
    jnz .Read3
    jmp .DiskError
.Done3:

    ret

.DiskError:
    mov si, msg_disk_err
    call print
    jmp $

BOOT_DISK: db 0
msg_disk_err: db "Disk read failed!", 0x0D, 0x0A, 0