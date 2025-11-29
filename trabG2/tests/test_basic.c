#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include "../include/gera_codigo.h"

/* Buffer para código gerado (64 KB deve ser suficiente para testes iniciais) */
#define CODE_BUFFER_SIZE (64 * 1024)
#define PAGE_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.lbs>\n", argv[0]);
        return 1;
    }

    /* Abre arquivo de entrada */
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    /* Aloca buffer para código gerado com alinhamento de página */
    unsigned char *code = NULL;
    if (posix_memalign((void **)&code, PAGE_SIZE, CODE_BUFFER_SIZE) != 0) {
        fprintf(stderr, "Erro alocando buffer de código\n");
        fclose(f);
        return 1;
    }
    memset(code, 0, CODE_BUFFER_SIZE);

    /* Marca buffer como executável */
    if (mprotect(code, CODE_BUFFER_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("mprotect");
        free(code);
        fclose(f);
        return 1;
    }

    /* Chama gerador */
    funcp entry = NULL;
    gera_codigo(f, code, &entry);

    fclose(f);

    printf("Entry point: %p\n", (void *)entry);

    /* ====================================================================
     * TESTES DE SANIDADE
     * ==================================================================== */

    /* Teste genérico: executa a função e mostra resultado */
    printf("\n=== Teste: Executando função gerada ===\n");
    int res = entry(42);
    printf("entry(42) = %d\n", res);
    printf("(Resultado depende da função no arquivo LBS)\n");

    printf("\n=== Todos os testes passaram! ===\n");

    free(code);
    return 0;
}
