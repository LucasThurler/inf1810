#include "bigint.h"
#include <stdio.h>
#include <string.h>

/*
 * Testes focados em big_val:
 * - Checar extensão de sinal (negativos) e ordem de bytes (little-endian).
 * - Casos: valores pequenos e limites LONG_MAX/LONG_MIN.
 */

void test_big_val() {
    BigInt res;

    // Teste 1: Valor positivo pequeno
    long val1 = 1;
    big_val(res, val1);
    printf("Teste 1: ");
    big_dump(res);

    // Teste 2: Valor negativo pequeno
    long val2 = -1;
    big_val(res, val2);
    printf("Teste 2: ");
    big_dump(res);

    // Teste 3: Valor positivo grande
    long val3 = 9223372036854775807; // LONG_MAX
    big_val(res, val3);
    printf("Teste 3: ");
    big_dump(res);

    // Teste 4: Valor negativo grande
    long val4 = -9223372036854775807 - 1; // LONG_MIN
    big_val(res, val4);
    printf("Teste 4: ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_val:\n");
    test_big_val();
    return 0;
}