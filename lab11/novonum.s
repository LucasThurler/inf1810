    .data
fmt_numero: .string "numero: "
fmt_scan:   .string "%d"

    .text
    .globl novonum
novonum:
    pushq %rbp          # salva base da pilha anterior (valor antigo de rbp)
    movq %rsp, %rbp     # define a base do novo registro de ativação
    subq $16, %rsp      # reserva espaço para vairáveis locais

    # printf("numero: ");
    leaq fmt_numero(%rip), %rdi # primeiro argumento: endereço da string
    call printf

    # scanf("%d",&minhalocal);
    leaq fmt_scan(%rip), %rdi   # primeiro argumento: formato "%d"
    leaq -4(%rbp), %rsi         # segundo argumento: endereco da variavel local
    call scanf

    # retorno: valor lido
    movl -4(%rbp), %eax         # move minhalocal para %eax (retorno)

    movq %rbp, %rsp
    popq %rbp
    ret