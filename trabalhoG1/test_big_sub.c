#include "bigint.h"
#include <stdio.h>
#include <string.h>

/*
 * Testes de subtração (big_sub):
 * - Validamos casos básicos e com sinais, usando comp2(b) + soma.
 * - Limites com LONG_MAX/MIN ajudam a checar borrow entre bytes.
 */

void test_big_sub() {
    BigInt res, a, b;

    // Teste 1: Subtração de 0 - 0
    big_val(a, 0);
    big_val(b, 0);
    big_sub(res, a, b);
    printf("Teste 1 (0 - 0): ");
    big_dump(res);

    // Teste 2: Subtração de 1 - 1
    big_val(a, 1);
    big_val(b, 1);
    big_sub(res, a, b);
    printf("Teste 2 (1 - 1): ");
    big_dump(res);

    // Teste 3: Subtração de -1 - 1
    big_val(a, -1);
    big_val(b, 1);
    big_sub(res, a, b);
    printf("Teste 3 (-1 - 1): ");
    big_dump(res);

    // Teste 4: Subtração de LONG_MAX - 1
    big_val(a, 9223372036854775807); // LONG_MAX
    big_val(b, 1);
    big_sub(res, a, b);
    printf("Teste 4 (LONG_MAX - 1): ");
    big_dump(res);

    // Teste 5: Subtração de LONG_MIN - (-1)
    big_val(a, -9223372036854775807 - 1); // LONG_MIN
    big_val(b, -1);
    big_sub(res, a, b);
    printf("Teste 5 (LONG_MIN - -1): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_sub:\n");
    test_big_sub();
    return 0;
}