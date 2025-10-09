#include "bigint.h"
#include <stdio.h>
#include <string.h>

void test_big_comp2() {
    BigInt res, a;

    // Teste 1: Complemento a 2 de 0
    big_val(a, 0);
    big_comp2(res, a);
    printf("Teste 1 (0): ");
    big_dump(res);

    // Teste 2: Complemento a 2 de 1
    big_val(a, 1);
    big_comp2(res, a);
    printf("Teste 2 (1): ");
    big_dump(res);

    // Teste 3: Complemento a 2 de -1
    big_val(a, -1);
    big_comp2(res, a);
    printf("Teste 3 (-1): ");
    big_dump(res);

    // Teste 4: Complemento a 2 de um valor grande
    big_val(a, 9223372036854775807); // LONG_MAX
    big_comp2(res, a);
    printf("Teste 4 (LONG_MAX): ");
    big_dump(res);

    // Teste 5: Complemento a 2 de um valor negativo grande
    big_val(a, -9223372036854775807 - 1); // LONG_MIN
    big_comp2(res, a);
    printf("Teste 5 (LONG_MIN): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_comp2:\n");
    test_big_comp2();
    return 0;
}