#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#define _XOPEN_SOURCE 600

typedef int (*funcp)(int);

int main() {
    unsigned char code[100];
    int off = 0;
    
    /* Bytecode da função */
    code[off++] = 0x55;                 /* push rbp */
    code[off++] = 0x48;                 /* mov rsp, rbp */
    code[off++] = 0x89;
    code[off++] = 0xe5;
    code[off++] = 0x48;                 /* sub $8, rsp */
    code[off++] = 0x83;
    code[off++] = 0xec;
    code[off++] = 0x08;
    code[off++] = 0x89;                 /* mov %edi, %eax */
    code[off++] = 0xf8;
    code[off++] = 0x83;                 /* add $1, %eax */
    code[off++] = 0xc0;
    code[off++] = 0x01;
    code[off++] = 0x89;                 /* mov %eax, -8(%rbp) */
    code[off++] = 0x45;
    code[off++] = 0xf8;
    code[off++] = 0x8b;                 /* mov -8(%rbp), %eax */
    code[off++] = 0x45;
    code[off++] = 0xf8;
    code[off++] = 0x5d;                 /* pop rbp */
    code[off++] = 0xc3;                 /* ret */
    
    /* Make code executable */
    unsigned char *code_aligned;
    if (posix_memalign((void **)&code_aligned, 4096, 4096) != 0) {
        perror("memalign");
        return 1;
    }
    memcpy(code_aligned, code, off);
    
    if (mprotect(code_aligned, 4096, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("mprotect");
        return 1;
    }
    
    funcp f = (funcp)code_aligned;
    int result = f(10);
    printf("f(10) = %d (expected 11)\n", result);
    
    free(code_aligned);
    return 0;
}
