/*

struct exam { f
    float val; 
    int tipo;
};


double inv (double val);

float boo (double d, float bias) {

    int i = 0;
    
    struct exam temp[5];
    
    //for (i=0; i<5; i++) {
    while(i<5){

        dobule ret = inv(d);

        temp[i].val = ret;
        temp[i].tipo = i;
        i++;
    }
    
    return temp[0].val + bias;
}

*/




/*
Dicionário

Reg     Var
ebx     i - CS
r12     ENDEREÇO aponta para o início o vetor de struct - CS
r13d    sizeof(struct exam) - CS
r14     i*r13d para 8 bytes - CS
xmm0    double d
xmm1    float bias
*/




.text 
.globl boo


boo:
    pushq %rbp
    movq %rsp, %rbp
    subq $96, %rsp


    /*Devo guardar os reg CS*/

    movq %rbx, -8(%rbp)
    movq %r12, -16(%rbp)
    movq %r13, -24(%rbp)
    movq %r14, -32(%rbp)

    /*Devo guardar os reg Caller S*/

    movsd %xmm0, -40(%rbp)
    movss %xmm1, -44(%rbp)

    movl $0, %ebx

    leaq -96(%rbp), %r12

    loop:
        cmpl $5, %ebx
        jge fora_loop

        call inv /*Não ta escrito aqui, mas acredite, o valor de retorno é o xmm0*/

        movl $8, %r13d
        imull %ebx, %r13d /* r13d = ebx * r13d */
        movslq %r13d, %r14
        addq %r14, %r12

        /* temp[i].val = xmm0 */
        cvtsd2ss %xmm0, %xmm0
        movss %xmm0, (%r12)
        movl %ebx, 4(%r12)

        /*addq $4, %r12
        movl %ebx, (%r12) Isso aqui é igual a movl %ebx, 4(%r12) mas em duas linhas*/
        
        incl %ebx

        fora_loop:

         return temp[0].val + bias;
        leaq -96(%rbp), %r12

        movss %(r12), %xmm0
        movss -44(%rbp), %xmm1

        addss %xmm1, %xmm0

        movq -8(%rbp), %rbx 
        movq -16(%rbp), %r12 
        movq -24(%rbp), %r13
        movq -32(%rbp), %r14

        leave
        ret 





