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
    int bytes_generated = gera_codigo(f, code, &entry);

    fclose(f);

    if (bytes_generated < 0) {
        fprintf(stderr, "Erro na geração de código\n");
        free(code);
        return 1;
    }

    printf("Código gerado: %d bytes\n", bytes_generated);
    printf("Entry point: %p\n", (void *)entry);

    if (!entry) {
        fprintf(stderr, "Erro: entry point não foi definido\n");
        free(code);
        return 1;
    }

    /* ====================================================================
     * TESTES DE SANIDADE
     * ==================================================================== */

    /* Teste 1: Primeira função (retorna constante $100)
     * Por enquanto, o stub retorna 0
     */
    printf("\n=== Teste 1: Função que retorna constante $100 ===\n");
    int res = entry(42);
    printf("entry(42) = %d (esperado: 0 no stub)\n", res);
    assert(res == 0); /* TODO: mudar para 100 após implementar emitter */

    printf("\n=== Todos os testes passaram! ===\n");

    free(code);
    return 0;
}
