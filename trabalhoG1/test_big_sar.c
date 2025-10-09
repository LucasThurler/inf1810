#include "bigint.h"
#include <stdio.h>
#include <string.h>

void test_big_sar() {
    BigInt res, a;

    // Teste 1: Deslocamento aritmético de 0 >> 1
    big_val(a, 0);
    big_sar(res, a, 1);
    printf("Teste 1 (0 >> 1): ");
    big_dump(res);

    // Teste 2: Deslocamento aritmético de 1 >> 1
    big_val(a, 1);
    big_sar(res, a, 1);
    printf("Teste 2 (1 >> 1): ");
    big_dump(res);

    // Teste 3: Deslocamento aritmético de -1 >> 1
    big_val(a, -1);
    big_sar(res, a, 1);
    printf("Teste 3 (-1 >> 1): ");
    big_dump(res);
}

int main() {
    printf("Iniciando testes para big_sar:\n");
    test_big_sar();
    return 0;
}