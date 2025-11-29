/*
 * Cabeçalho para o 2º trabalho
 * Lembre-se de ***NÃO*** entregar a função main
 * ***NÃO*** modifique esse arquivo
 */

#ifndef GERA_CODIGO_H
#define GERA_CODIGO_H

#include <stdio.h>

/* tipo ponteiro para função que recebe um inteiro e retorna um inteiro */
typedef int (*funcp) (int x);

/* protótipo da função que recebe um arquivo previamente aberto e preenche 
 * o vetor de bytes (char) com o código gerado
 */
void gera_codigo (FILE *f, unsigned char code[], funcp *entry);

#endif
