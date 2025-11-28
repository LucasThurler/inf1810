    .text
    .globl foo
    .type foo, @function

foo:
    # -primeiro argumento em %edi
    # -valor de retorno em %executa
    # queremos retornar x+1

    pushq %rbp
    movq %rsp, %rbp
    movl %edi, %eax
    addl $1, %eax
    popq %rbp
    ret

