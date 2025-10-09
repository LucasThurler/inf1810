#include <stdio.h>
#include <string.h>
#include "bigint.h"

  /* Lucas Thurler Gonçalves 2212824 3WA */
  /* João Pedro Mezian 2410625 3WA */

/* Atribuição: res = val (extensão com sinal para 128 bits, little-endian) */
void big_val(BigInt res, long val) {
    // Zera todos os bytes
    unsigned char fill = (val < 0) ? 0xFF : 0x00;
    for (int i = 0; i < NUM_BITS/8; i++) res[i] = fill;
    // Copia os bytes de val para res (little-endian)
    for (int i = 0; i < sizeof(long); i++) {
        res[i] = (val >> (8 * i)) & 0xFF;
    }
}

/* Função auxiliar para dump do BigInt (para testes/depuração) */
void big_dump(BigInt a) {
    for (int i = 0; i < NUM_BITS/8; i++) {
        printf("%02X ", a[i]);
    }
    printf("\n");
}

/* Complemento a 2: res = -a (inverte bits e soma 1) */
void big_comp2(BigInt res, BigInt a) {
    unsigned char carry = 1;

    // Inverte os bits
    for (int i = 0; i < NUM_BITS / 8; i++) {
        res[i] = ~a[i];
    }

    // Soma 1 para obter o complemento a 2
    for (int i = 0; i < NUM_BITS / 8; i++) {
        unsigned short temp = res[i] + carry;
        res[i] = temp & 0xFF;
        carry = (temp >> 8) & 0x1;
    }
}

/* Soma: res = a + b (soma byte a byte com carry) */
void big_sum(BigInt res, BigInt a, BigInt b) {
    unsigned char carry = 0;

    for (int i = 0; i < NUM_BITS / 8; i++) {
        unsigned short temp = a[i] + b[i] + carry;
        res[i] = temp & 0xFF;
        carry = (temp >> 8) & 0x1;
    }
}

/* Subtração: res = a - b (usa comp2(b) e soma) */
void big_sub(BigInt res, BigInt a, BigInt b) {
    BigInt b_neg;
    big_comp2(b_neg, b); // Calcula o complemento a 2 de b
    big_sum(res, a, b_neg); // Soma a com o complemento a 2 de b
}

/*
 * Multiplicação: res = a * b
 * Estratégia simples: percorremos os 128 bits de b; para cada bit 1,
 * somamos (a << i) em um acumulador. Observação: isso é adequado para
 * interpretação não assinada. Para negativos, o resultado pode não bater
 * com a semântica de multiplicação assinada completa (sem Booth).
 */
void big_mul(BigInt res, BigInt a, BigInt b) {
    BigInt temp_res = {0};
    BigInt temp_a;
    memcpy(temp_a, a, NUM_BITS / 8);

    for (int i = 0; i < NUM_BITS; i++) {
        if (b[i / 8] & (1 << (i % 8))) {
            BigInt shifted_a;
            big_shl(shifted_a, temp_a, i);
            big_sum(temp_res, temp_res, shifted_a);
        }
    }

    memcpy(res, temp_res, NUM_BITS / 8);
}

/* Deslocamento à esquerda: res = a << n (lógico). Se n >= 128, zera. */
void big_shl(BigInt res, BigInt a, int n) {
    memset(res, 0, NUM_BITS / 8);

    int byte_shift = n / 8;
    int bit_shift = n % 8;

    for (int i = 0; i < NUM_BITS / 8; i++) {
        if (i + byte_shift < NUM_BITS / 8) {
            res[i + byte_shift] |= a[i] << bit_shift;
            if (i + byte_shift + 1 < NUM_BITS / 8 && bit_shift > 0) {
                res[i + byte_shift + 1] |= a[i] >> (8 - bit_shift);
            }
        }
    }
}

/* Deslocamento lógico à direita: res = a >> n. Se n >= 128, zera. */
void big_shr(BigInt res, BigInt a, int n) {
    memset(res, 0, NUM_BITS / 8);

    int byte_shift = n / 8;
    int bit_shift = n % 8;

    for (int i = 0; i < NUM_BITS / 8; i++) {
        if (i - byte_shift >= 0) {
            res[i - byte_shift] |= a[i] >> bit_shift;
            if (i - byte_shift - 1 >= 0 && bit_shift > 0) {
                res[i - byte_shift - 1] |= a[i] << (8 - bit_shift);
            }
        }
    }
}

/*
 * Deslocamento aritmético à direita: res = a >> n (preserva sinal).
 * Se n >= 128, preenchimento total: 0xFF quando negativo, 0 quando positivo.
 * Nota: quando n não é múltiplo de 8, a propagação do bit de sinal pode
 * exigir atenção nos limites entre bytes.
 */
void big_sar(BigInt res, BigInt a, int n) {
    memset(res, 0, NUM_BITS / 8);

    int byte_shift = n / 8;
    int bit_shift = n % 8;
    unsigned char sign = (a[NUM_BITS / 8 - 1] & 0x80) ? 0xFF : 0x00;

    for (int i = 0; i < NUM_BITS / 8; i++) {
        if (i - byte_shift >= 0) {
            res[i - byte_shift] |= a[i] >> bit_shift;
            if (i - byte_shift - 1 >= 0 && bit_shift > 0) {
                res[i - byte_shift - 1] |= a[i] << (8 - bit_shift);
            }
        }
    }

    // Preenche os bits de sinal
    for (int i = NUM_BITS / 8 - byte_shift; i < NUM_BITS / 8; i++) {
        res[i] = sign;
    }
}