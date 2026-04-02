%macro IncBin 2
    SECTION .rodata
    GLOBAL %1           ; If your project uses the GLOBAL macro, pass %1 here

%1:                     ; The label itself
    incbin %2           ; Pull in the file
    db 0                ; Null terminator
    %1_size: dq $ - %1  ; Use '$' (current position) - start to get the size
%endmacro

; Usage: Make sure there is a COMMA here!
IncBin Test, "logo.txt"