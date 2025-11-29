.globl ret_param
ret_param:
    push %rbp
    mov %rsp, %rbp
    mov %edi, %eax
    pop %rbp
    ret
