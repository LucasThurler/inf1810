#include "bigint.h"
#include <stdio.h>
#include <string.h>

void test_big_mul() {
    BigInt res, a, b;

    // Teste 1: Multiplicação de 0 * 0
    big_val(a, 0);
    big_val(b, 0);
    big_mul(res, a, b);
    printf("Teste 1 (0 * 0): ");
    big_dump(res);

    // Teste 2: Multiplicação de 1 * 1
    big_val(a, 1);
    big_val(b, 1);
    big_mul(res, a, b);
    printf("Teste 2 (1 * 1): ");
    big_dump(res);

    // Teste 3: Multiplicação de -1 * 1
    big_val(a, -1);
    big_val(b, 1);
    big_mul(res, a, b);
    printf("Teste 3 (-1 * 1): ");
    big_dump(res);

    // Teste 4: Multiplicação de LONG_MAX * 2
    big_val(a, 9223372036854775807); // LONG_MAX
    big_val(b, 2);
    big_mul(res, a, b);
    printf("Teste 4 (LONG_MAX * 2): ");
    big_dump(res);

    // Teste 5: Multiplicação de LONG_MIN * -1
    big_val(a, -9223372036854775807 - 1); // LONG_MIN
    big_val(b, -1);
    big_mul(res, a, b);
    printf("Teste 5 (LONG_MIN * -1): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_mul:\n");
    test_big_mul();
    return 0;
}