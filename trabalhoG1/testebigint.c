#include <stdio.h>
#include "bigint.h"

  /* Lucas Thurler Gonçalves 2212824 3WA */
  /* João Pedro Mezian 2410625 3WA */

int main(void) {
    BigInt a, b, r;

    // Teste de inicialização
    big_val(a, 25);
    big_val(b, 10);

    printf("A = "); big_dump(a); printf("\n");
    printf("B = "); big_dump(b); printf("\n");

    // Soma
    big_sum(r, a, b);
    printf("A + B = "); big_dump(r); printf("\n");

    // Subtração
    big_sub(r, a, b);
    printf("A - B = "); big_dump(r); printf("\n");

    // Multiplicação
    big_mul(r, a, b);
    printf("A * B = "); big_dump(r); printf("\n");

    // Deslocamento lógico à esquerda
    big_shl(r, a, 1);
    printf("A << 1 = "); big_dump(r); printf("\n");

    // Deslocamento lógico à direita
    big_shr(r, a, 1);
    printf("A >> 1 = "); big_dump(r); printf("\n");

    // Deslocamento aritmético à direita
    big_sar(r, a, 1);
    printf("A >>> 1 (SAR) = "); big_dump(r); printf("\n");

    return 0;
}
