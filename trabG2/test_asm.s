.globl myfunc
myfunc:
    push %rbp
    mov %rsp, %rbp
    mov $100, %eax
    pop %rbp
    ret
