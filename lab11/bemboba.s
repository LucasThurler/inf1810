    .text
    .globl bemboba
bemboba:
    # Prólogo
    pushq %rbp
    movq  %rsp, %rbp
    subq  $32, %rsp          # espaço para local[4] (4 ints = 16 bytes) + alinhamento

    # i = 0, a = local
    movl  $0, %ecx           # i em %ecx
    leaq  -16(%rbp), %r8     # endereço de local[0] -> %r8 = a

loop_inicio:
    cmpl  $4, %ecx           # compara i com 4
    jge   fim_loop

    # *a = num
    movl  %edi, (%r8)        # *a = num

    # a++
    addq  $4, %r8            # avança ponteiro (4 bytes = sizeof(int))

    # i++
    incl  %ecx
    jmp   loop_inicio

fim_loop:
    # chamar addl(local, 4)
    leaq  -16(%rbp), %rdi    # 1º argumento: endereço de local
    movl  $4, %esi           # 2º argumento: 4
    call  addl

    # valor de retorno já está em %eax
    movq  %rbp, %rsp
    popq  %rbp
    ret
