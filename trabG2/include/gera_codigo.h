#ifndef GERA_CODIGO_H
#define GERA_CODIGO_H

#include <stdio.h>

/* Tipo de ponteiro para funções geradas dinamicamente.
 * Aceita um argumento inteiro de 32 bits e retorna inteiro de 32 bits.
 */
typedef int (*funcp)(int);

/* Gera código de máquina x86-64 a partir de um arquivo LBS.
 *
 * Parâmetros:
 *   f       - arquivo aberto para leitura contendo código LBS
 *   code    - buffer onde o código de máquina será escrito
 *   entry   - será preenchido com ponteiro para o início da última função
 *
 * Retorna:
 *   número de bytes escritos em code, ou -1 em caso de erro
 */
int gera_codigo(FILE *f, unsigned char code[], funcp *entry);

#endif
