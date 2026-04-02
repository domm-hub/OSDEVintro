PROGRAM_SPACE equ 0x8000

ReadDisk:
    mov [BOOT_DISK], dl
    xor ax, ax
    int 0x13           ; Reset disk
    jc .DiskError

    mov bx, PROGRAM_SPACE
    mov cl, 2          ; Sector 2 (Sector 1 is MBR)
    mov ch, 0          ; Cylinder 0
    mov dh, 0          ; Head 0
    mov di, 128        ; Total sectors to read (64KB / 512 = 128)

.ReadLoop:
    mov ah, 0x02
    mov al, 1          ; Read 1 sector at a time for safety across tracks
    mov dl, [BOOT_DISK]
    int 0x13
    jc .DiskError

    add bx, 512        ; Next buffer position
    inc cl             ; Next sector
    cmp cl, 19         ; If we hit sector 19, move to next head/cylinder
    jne .ContinueLoop

    mov cl, 1          ; Reset sector to 1
    inc dh             ; Next head
    cmp dh, 2          ; If we hit head 2, move to next cylinder
    jne .ContinueLoop

    mov dh, 0          ; Reset head to 0
    inc ch             ; Next cylinder

.ContinueLoop:
    dec di             ; One less sector to read
    jnz .ReadLoop
    ret

.DiskError:
    mov si, msg_disk_err
    call print
    jmp $

BOOT_DISK:
    db 0

msg_disk_err:
    db "Disk read failed!", 0x0D, 0x0A, 0
