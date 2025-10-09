#include "bigint.h"
#include <stdio.h>
#include <string.h>

void test_big_sum() {
    BigInt res, a, b;

    // Teste 1: Soma de 0 + 0
    big_val(a, 0);
    big_val(b, 0);
    big_sum(res, a, b);
    printf("Teste 1 (0 + 0): ");
    big_dump(res);

    // Teste 2: Soma de 1 + 1
    big_val(a, 1);
    big_val(b, 1);
    big_sum(res, a, b);
    printf("Teste 2 (1 + 1): ");
    big_dump(res);

    // Teste 3: Soma de -1 + 1
    big_val(a, -1);
    big_val(b, 1);
    big_sum(res, a, b);
    printf("Teste 3 (-1 + 1): ");
    big_dump(res);

    // Teste 4: Soma de LONG_MAX + 1
    big_val(a, 9223372036854775807); // LONG_MAX
    big_val(b, 1);
    big_sum(res, a, b);
    printf("Teste 4 (LONG_MAX + 1): ");
    big_dump(res);

    // Teste 5: Soma de LONG_MIN + (-1)
    big_val(a, -9223372036854775807 - 1); // LONG_MIN
    big_val(b, -1);
    big_sum(res, a, b);
    printf("Teste 5 (LONG_MIN + -1): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_sum:\n");
    test_big_sum();
    return 0;
}