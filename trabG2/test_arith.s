.globl arith_simple
arith_simple:
    push %rbp
    mov %rsp, %rbp
    sub $8, %rsp
    mov %edi, %eax
    add $1, %eax
    mov %eax, -8(%rbp)
    mov -8(%rbp), %eax
    pop %rbp
    ret
