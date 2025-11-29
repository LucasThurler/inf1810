.globl caller
.globl callee

callee:
    mov $5, %eax
    ret

caller:
    push %rbp
    mov %rsp, %rbp
    mov %edi, %eax
    call callee
    pop %rbp
    ret
