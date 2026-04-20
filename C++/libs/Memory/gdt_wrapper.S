.section .text
.global LoadGDT
.global LoadTR

LoadGDT:
    lgdt (%rdi)
    
    pushq $0x08             /* New CS selector */
    leaq .next(%rip), %rax  /* Return address */
    pushq %rax
    lretq                   /* Far return to update CS */
.next:
    mov $0x10, %ax          /* New Data selector */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret

LoadTR:
    ltr %di
    ret

.global LoadCR3
LoadCR3:
    mov %rdi, %cr3
    ret

.global InvalidatePage
InvalidatePage:
    invlpg (%rdi)
    ret
