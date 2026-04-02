print:
    pusha
    mov bh, 0          ; Page 0
    .Loop:
        lodsb              ; AL = [SI], SI++
        cmp al, 0          ; End of string?
        je .done
        mov ah, 0x0E       ; Teletype output
        int 0x10
        jmp .Loop
.done:
    popa
    ret
