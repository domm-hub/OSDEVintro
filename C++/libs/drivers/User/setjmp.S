.section .text
.global setjmp
.global longjmp

# ---------------------------------------------------------
# int setjmp(jmp_buf env)
# RDI = Pointer to the jmp_buf array
# ---------------------------------------------------------
setjmp:
    # Save the 6 callee-saved registers into the array
    movq %rbx, 0x00(%rdi)
    movq %rbp, 0x08(%rdi)
    movq %r12, 0x10(%rdi)
    movq %r13, 0x18(%rdi)
    movq %r14, 0x20(%rdi)
    movq %r15, 0x28(%rdi)

    # Save the Stack Pointer (RSP)
    # We add 8 to RSP because the 'call' instruction pushed the 
    # return address onto the stack. We want the stack as it was BEFORE the call.
    leaq 8(%rsp), %rax
    movq %rax, 0x30(%rdi)

    # Save the Instruction Pointer (RIP)
    # The return address is currently sitting at the very top of the stack.
    movq (%rsp), %rax
    movq %rax, 0x38(%rdi)

    # setjmp always returns 0 when first called
    xorq %rax, %rax 
    ret

# ---------------------------------------------------------
# void longjmp(jmp_buf env, int val)
# RDI = Pointer to the jmp_buf array
# RSI = The value to return (val)
# ---------------------------------------------------------
longjmp:
    # MicroPython (and the C standard) requires that if val is 0, 
    # we must return 1 instead. 
    movq %rsi, %rax
    testq %rax, %rax
    jnz .restore_state
    movq $1, %rax

.restore_state:
    # Restore the 6 callee-saved registers
    movq 0x00(%rdi), %rbx
    movq 0x08(%rdi), %rbp
    movq 0x10(%rdi), %r12
    movq 0x18(%rdi), %r13
    movq 0x20(%rdi), %r14
    movq 0x28(%rdi), %r15

    # Restore the Stack Pointer
    movq 0x30(%rdi), %rsp

    # Jump directly to the saved Instruction Pointer (RIP)
    # This tricks the CPU into thinking it just successfully returned from setjmp!
    jmpq *0x38(%rdi)
