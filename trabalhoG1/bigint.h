/*
 * Representação geral (resumo rápido):
 * - Usamos 128 bits no total, armazenados em um vetor de 16 bytes (unsigned char[16]).
 * - Ordem dos bytes: little-endian (byte 0 = menos significativo, byte 15 = mais significativo).
 * - Para valores negativos, consideramos complemento de dois no total de 128 bits.
 * - Sobre aliasing: as funções aceitam "res" igual a "a" ou "b" (quando faz sentido),
 *   pois sempre escrevemos o resultado byte a byte sem depender do valor já escrito.
 * - Sobre deslocamentos (shifts): se n >= 128, o resultado vira 0 para SHL/SHR; em SAR,
 *   o resultado fica todo 0xFF quando o bit de sinal é 1 (valor negativo), senão 0.
 */
#define NUM_BITS 128
typedef unsigned char BigInt[NUM_BITS/8];

/* Atribuicao */

/* res = val (extensão com sinal para 128 bits, respeitando little-endian) */
void big_val (BigInt res, long val);

/* Operacoes aritmeticas */

/* res = -a (complemento de dois em 128 bits) */
void big_comp2(BigInt res, BigInt a);

/* res = a + b (soma byte a byte com carry) */
void big_sum (BigInt res, BigInt a, BigInt b);

/* res = a - b (usa complemento de dois de b + soma) */
void big_sub (BigInt res, BigInt a, BigInt b);

/*
 * res = a * b
 * Obs.: implementação por soma de parciais com base nos bits de b (forma simples).
 * Para operandos negativos, o comportamento não é o de uma multiplicação assinada
 * "completa" (não usamos Booth). Vale testar casos com sinais.
 */
void big_mul (BigInt res, BigInt a, BigInt b);

/* Operacoes de deslocamento */

/* res = a << n (lógico). Se n >= 128, resultado = 0. */
void big_shl (BigInt res, BigInt a, int n);

/* res = a >> n (lógico). Se n >= 128, resultado = 0. */
void big_shr (BigInt res, BigInt a, int n);

/* res = a >> n (aritmético, preserva sinal). Se n >= 128, vira 0xFF se negativo, senão 0. */
void big_sar(BigInt res, BigInt a, int n);

/* Função auxiliar: imprime os 16 bytes em hexa (útil para debug) */
void big_dump(BigInt a);