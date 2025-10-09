#include "bigint.h"
#include <stdio.h>
#include <string.h>

void test_big_shl() {
    BigInt res, a;

    // Teste 1: Deslocamento de 0 << 1
    big_val(a, 0);
    big_shl(res, a, 1);
    printf("Teste 1 (0 << 1): ");
    big_dump(res);

    // Teste 2: Deslocamento de 1 << 1
    big_val(a, 1);
    big_shl(res, a, 1);
    printf("Teste 2 (1 << 1): ");
    big_dump(res);

    // Teste 3: Deslocamento de -1 << 1
    big_val(a, -1);
    big_shl(res, a, 1);
    printf("Teste 3 (-1 << 1): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_shl:\n");
    test_big_shl();
    return 0;
}