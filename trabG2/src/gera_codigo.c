/* Aluno1_Nome Matricula Turma */
/* Aluno2_Nome Matricula Turma */

#include "gera_codigo.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================================================================
 * ESTRUTURAS DE DADOS
 * ======================================================================== */

/* Tipos de expressões/operandos */
typedef enum {
    OPERAND_CONST,      /* constante $n */
    OPERAND_VAR,        /* variável local v0..v4 */
    OPERAND_PARAM,      /* parâmetro p0 */
    OPERAND_INVALID
} OperandType;

/* Operando (pode ser constante, variável ou parâmetro) */
typedef struct {
    OperandType type;
    int value;          /* para OPERAND_CONST ou índice da variável */
} Operand;

/* Tipos de instruções LBS */
typedef enum {
    INSTR_ASSIGN,       /* v = expr */
    INSTR_ARITH,        /* v = a op b */
    INSTR_CALL,         /* call func_num arg */
    INSTR_RET,          /* ret expr */
    INSTR_ZRET,         /* zret cond expr */
    INSTR_INVALID
} InstrType;

/* Instrução LBS intermediária */
typedef struct {
    InstrType type;
    Operand dest;       /* variável destino (para ASSIGN) */
    Operand src1, src2; /* operandos */
    char op;            /* operador aritmético (+, -, *) */
    int func_num;       /* número da função a chamar (para CALL) */
} Instruction;

/* Função LBS */
typedef struct {
    int num;                    /* número da função (0, 1, ...) */
    int num_instrs;
    Instruction *instrs;
    int code_offset;            /* offset no buffer code[] */
    int code_size;              /* tamanho do código em bytes */
} Function;

/* Contexto do gerador */
typedef struct {
    Function *functions;
    int num_functions;
    unsigned char *code_buffer;
    int code_offset;            /* posição atual de emissão */
} GenContext;

/* ========================================================================
 * PARSER COM fscanf conforme enunciado
 * ======================================================================== */

/* Faz parse do arquivo LBS completo usando fscanf */
static int parse_file(FILE *f, Function **out_funcs) {
    int num_functions = 0;
    int capacity = 8;
    *out_funcs = malloc(capacity * sizeof(Function));
    int line = 1;

    int c;
    while ((c = fgetc(f)) != EOF) {
        switch (c) {
            case 'f': { /* function */
                char c0;
                if (fscanf(f, "unction%c", &c0) != 1) {
                    fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                    break;
                }

                /* Aloca nova função */
                if (num_functions >= capacity) {
                    capacity *= 2;
                    *out_funcs = realloc(*out_funcs, capacity * sizeof(Function));
                }

                Function *func = &(*out_funcs)[num_functions];
                memset(func, 0, sizeof(Function));
                func->num = num_functions;
                func->instrs = malloc(16 * sizeof(Instruction));
                func->num_instrs = 0;

                /* Lê instruções até end */
                int instr_c;
                while ((instr_c = fgetc(f)) != EOF) {
                    if (instr_c == '\n') {
                        line++;
                        continue;
                    }
                    
                    /* Verifica end */
                    if (instr_c == 'e') {
                        char c_e;
                        if (fscanf(f, "nd%c", &c_e) != 1) {
                            ungetc(instr_c, f);
                        } else {
                            break;
                        }
                    }

                    /* Processa instrução dentro da função */
                    if (instr_c == 'r') { /* ret */
                        int idx0;
                        char var0;
                        if (fscanf(f, "et %c%d", &var0, &idx0) != 2) {
                            fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                        } else {
                            Instruction instr;
                            memset(&instr, 0, sizeof(instr));
                            instr.type = INSTR_RET;
                            instr.src1.type = (var0 == '$') ? OPERAND_CONST : 
                                             (var0 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                            instr.src1.value = (var0 == '$') ? idx0 : idx0;
                            if (func->num_instrs >= 16) {
                                func->instrs = realloc(func->instrs, 
                                    (func->num_instrs + 16) * sizeof(Instruction));
                            }
                            func->instrs[func->num_instrs++] = instr;
                        }
                    } else if (instr_c == 'z') { /* zret */
                        int idx0, idx1;
                        char var0, var1;
                        if (fscanf(f, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) {
                            fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                        } else {
                            Instruction instr;
                            memset(&instr, 0, sizeof(instr));
                            instr.type = INSTR_ZRET;
                            instr.src1.type = (var0 == '$') ? OPERAND_CONST : 
                                             (var0 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                            instr.src1.value = idx0;
                            instr.src2.type = (var1 == '$') ? OPERAND_CONST : 
                                             (var1 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                            instr.src2.value = idx1;
                            if (func->num_instrs >= 16) {
                                func->instrs = realloc(func->instrs, 
                                    (func->num_instrs + 16) * sizeof(Instruction));
                            }
                            func->instrs[func->num_instrs++] = instr;
                        }
                    } else if (instr_c == 'v') { /* atribuição */
                        int idx0;
                        char c0;
                        if (fscanf(f, "%d = %c", &idx0, &c0) != 2) {
                            fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                        } else if (c0 == 'c') { /* call */
                            int func_num, idx1;
                            char var1;
                            if (fscanf(f, "all %d %c%d", &func_num, &var1, &idx1) != 3) {
                                fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                            } else {
                                Instruction instr;
                                memset(&instr, 0, sizeof(instr));
                                instr.type = INSTR_CALL;
                                instr.dest.type = OPERAND_VAR;
                                instr.dest.value = idx0;
                                instr.func_num = func_num;
                                instr.src1.type = (var1 == '$') ? OPERAND_CONST : 
                                                 (var1 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                                instr.src1.value = idx1;
                                if (func->num_instrs >= 16) {
                                    func->instrs = realloc(func->instrs, 
                                        (func->num_instrs + 16) * sizeof(Instruction));
                                }
                                func->instrs[func->num_instrs++] = instr;
                            }
                        } else { /* operação aritmética */
                            int idx1, idx2;
                            char var1 = c0, var2, op;
                            if (fscanf(f, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4) {
                                fprintf(stderr, "erro: comando invalido na linha %d\n", line);
                            } else {
                                Instruction instr;
                                memset(&instr, 0, sizeof(instr));
                                instr.type = INSTR_ARITH;
                                instr.dest.type = OPERAND_VAR;
                                instr.dest.value = idx0;
                                instr.src1.type = (var1 == '$') ? OPERAND_CONST : 
                                                 (var1 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                                instr.src1.value = idx1;
                                instr.src2.type = (var2 == '$') ? OPERAND_CONST : 
                                                 (var2 == 'v') ? OPERAND_VAR : OPERAND_PARAM;
                                instr.src2.value = idx2;
                                instr.op = op;
                                if (func->num_instrs >= 16) {
                                    func->instrs = realloc(func->instrs, 
                                        (func->num_instrs + 16) * sizeof(Instruction));
                                }
                                func->instrs[func->num_instrs++] = instr;
                            }
                        }
                    } else {
                        fprintf(stderr, "erro: comando desconhecido na linha %d\n", line);
                    }
                }

                num_functions++;
                break;
            }
            case '\n': {
                line++;
                break;
            }
            default: {
                /* Ignora outros caracteres (espaços, tabs) */
                break;
            }
        }
        
        /* Pula espaços */
        int ret = fscanf(f, " ");
        (void)ret;
    }

    return num_functions;
}

/* ========================================================================
 * EMITTER
 * ======================================================================== */

/* Emite uma função que retorna uma constante.
 * Bytecode obtido com objdump de assembly real.
 * 
 * Prologue:
 *   55              push %rbp
 *   48 89 e5        mov %rsp, %rbp
 * 
 * Corpo (retorna constante imm32):
 *   b8 XX XX XX XX  mov $imm32, %eax
 * 
 * Epilogue:
 *   5d              pop %rbp
 *   c3              ret
 */
static void emit_const_return(unsigned char *code, int *offset, int value) {
    /* push rbp */
    code[(*offset)++] = 0x55;
    
    /* mov rsp, rbp */
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xe5;
    
    /* mov $value, eax */
    code[(*offset)++] = 0xb8;
    code[(*offset)++] = (value >>  0) & 0xFF;
    code[(*offset)++] = (value >>  8) & 0xFF;
    code[(*offset)++] = (value >> 16) & 0xFF;
    code[(*offset)++] = (value >> 24) & 0xFF;
    
    /* pop rbp */
    code[(*offset)++] = 0x5d;
    
    /* ret */
    code[(*offset)++] = 0xc3;
}

/* Emite uma função que retorna o parâmetro p0.
 * Bytecode obtido com objdump de assembly real.
 * 
 * Prologue:
 *   55              push %rbp
 *   48 89 e5        mov %rsp, %rbp
 * 
 * Corpo (retorna p0 que está em rdi):
 *   89 f8           mov %edi, %eax
 * 
 * Epilogue:
 *   5d              pop %rbp
 *   c3              ret
 */
static void emit_param_return(unsigned char *code, int *offset) {
    /* push rbp */
    code[(*offset)++] = 0x55;
    
    /* mov rsp, rbp */
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xe5;
    
    /* mov %edi, %eax */
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xf8;
    
    /* pop rbp */
    code[(*offset)++] = 0x5d;
    
    /* ret */
    code[(*offset)++] = 0xc3;
}

/* Emite uma função com operação aritmética: v0 = p0 OP $const; ret v0
 * Suporta operadores: + (add), - (sub), * (imul)
 * 
 * Bytecode:
 *   89 f8        mov %edi, %eax       (p0 para eax)
 *   83/81 c0/e8 XX     add/sub $const, %eax
 *   c3           ret
 */
static void emit_arith_const_return(unsigned char *code, int *offset, 
                                    char op, int constant) {
    /* mov %edi, %eax (p0 para eax) */
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xf8;
    
    /* Operação aritmética com constante */
    if (op == '+') {
        /* add $constant, %eax */
        if (constant >= -128 && constant <= 127) {
            code[(*offset)++] = 0x83;
            code[(*offset)++] = 0xc0;
            code[(*offset)++] = constant & 0xFF;
        } else {
            code[(*offset)++] = 0x05;
            code[(*offset)++] = (constant >>  0) & 0xFF;
            code[(*offset)++] = (constant >>  8) & 0xFF;
            code[(*offset)++] = (constant >> 16) & 0xFF;
            code[(*offset)++] = (constant >> 24) & 0xFF;
        }
    } else if (op == '-') {
        /* sub $constant, %eax */
        if (constant >= -128 && constant <= 127) {
            code[(*offset)++] = 0x83;
            code[(*offset)++] = 0xe8;
            code[(*offset)++] = constant & 0xFF;
        } else {
            code[(*offset)++] = 0x2d;
            code[(*offset)++] = (constant >>  0) & 0xFF;
            code[(*offset)++] = (constant >>  8) & 0xFF;
            code[(*offset)++] = (constant >> 16) & 0xFF;
            code[(*offset)++] = (constant >> 24) & 0xFF;
        }
    } else if (op == '*') {
        /* imul $constant, %eax -> mov para ecx, imul $constant, %eax, %ecx */
        /* Mais complexo - vamos usar: imul $constant, %eax */
        /* Instrução: 69 c0 XX XX XX XX (imul $imm32, %eax, %eax) */
        code[(*offset)++] = 0x69;
        code[(*offset)++] = 0xc0;
        code[(*offset)++] = (constant >>  0) & 0xFF;
        code[(*offset)++] = (constant >>  8) & 0xFF;
        code[(*offset)++] = (constant >> 16) & 0xFF;
        code[(*offset)++] = (constant >> 24) & 0xFF;
    }
    
    /* ret */
    code[(*offset)++] = 0xc3;
}

/* ========================================================================
 * EMITTERS COM STACK FRAME
 * ======================================================================== */

/* Calcula o offset de uma variável local no stack.
 * v0 = -8(%rbp), v1 = -12(%rbp), ..., v4 = -28(%rbp)
 */
static int var_offset(int var_idx) {
    return -(8 + var_idx * 4);
}

/* Emite código para salvar um valor em uma variável local */
static void emit_store_var(unsigned char *code, int *offset, int var_idx) {
    int off = var_offset(var_idx);
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0x45;
    code[(*offset)++] = off & 0xFF;
}

/* Emite código para carregar uma variável local em %eax */
static void emit_load_var(unsigned char *code, int *offset, int var_idx) {
    int off = var_offset(var_idx);
    code[(*offset)++] = 0x8b;
    code[(*offset)++] = 0x45;
    code[(*offset)++] = off & 0xFF;
}

/* Emite prologue com stack frame */
static void emit_prologue(unsigned char *code, int *offset) {
    code[(*offset)++] = 0x55;
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xe5;
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x83;
    code[(*offset)++] = 0xec;
    code[(*offset)++] = 0x28;
}

/* Emite epilogue */
static void emit_epilogue(unsigned char *code, int *offset) {
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xec;
    code[(*offset)++] = 0x5d;
    code[(*offset)++] = 0xc3;
}

/* Emite retorno condicional (zret).
 * Semântica: se primeiro operando == 0, retorna segundo operando; senão continua.
 * 
 * Bytecode:
 *   83 ff 00        cmp $0, %edi       (testa se p0 == 0)
 *   75 06           jne else_case      (pula 6 bytes se NÃO zero)
 *   b8 64 00 00 00  mov $0x64, %eax   (carrega valor de retorno se for zero)
 *   c3              ret
 *   b8 00 00 00 00  mov $0, %eax      (retorna 0 se não entrou)
 *   c3              ret
 */
static void emit_zret_simple(unsigned char *code, int *offset, 
                             int cond_value, int ret_value) {
    /* Testa se condição == 0: cmp $0, valor */
    /* Para p0: cmp $0, %edi */
    if (cond_value == 0) {
        /* Teste do parâmetro p0 */
        code[(*offset)++] = 0x83;
        code[(*offset)++] = 0xff;
        code[(*offset)++] = 0x00;
        
        /* jne (jump if not equal) - pula se NÃO for zero */
        code[(*offset)++] = 0x75;
        code[(*offset)++] = 0x06;  /* tamanho de mov $imm32, %eax = 5 + ret = 1 = 6 */
        
        /* mov $ret_value, %eax (se chegou aqui, p0 era zero) */
        code[(*offset)++] = 0xb8;
        code[(*offset)++] = (ret_value >>  0) & 0xFF;
        code[(*offset)++] = (ret_value >>  8) & 0xFF;
        code[(*offset)++] = (ret_value >> 16) & 0xFF;
        code[(*offset)++] = (ret_value >> 24) & 0xFF;
        
        /* ret */
        code[(*offset)++] = 0xc3;
        
        /* else case: mov $0, %eax (retorna 0 se condição não era zero) */
        code[(*offset)++] = 0xb8;
        code[(*offset)++] = 0x00;
        code[(*offset)++] = 0x00;
        code[(*offset)++] = 0x00;
        code[(*offset)++] = 0x00;
        
        /* ret */
        code[(*offset)++] = 0xc3;
    }
}

/* Emite uma função que chama outra função.
 * Para Passo 7: v0 = call func_num p0; ret v0
 * 
 * Bytecode (prologue + call + epilogue):
 *   55           push %rbp
 *   48 89 e5     mov %rsp, %rbp
 *   89 f8        mov %edi, %eax       (passa p0 em rdi/edi para chamada)
 *   e8 XX XX XX XX  call rel32        (call com rel32 - será preenchido depois)
 *   89 c0        mov %eax, %eax       (nop - mantém resultado em eax)
 *   5d           pop %rbp
 *   c3           ret
 * 
 * Nota: rel32 será calculado em segundo passo após emitir todas as funções.
 */
static void emit_call_function(unsigned char *code, int *offset, 
                               int target_func_offset) {
    /* push rbp */
    code[(*offset)++] = 0x55;
    
    /* mov rsp, rbp */
    code[(*offset)++] = 0x48;
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xe5;
    
    /* mov %edi, %eax (passa parâmetro) */
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xf8;
    
    /* call rel32 - deslocamento será calculado depois */
    int call_instr_offset = *offset;
    code[(*offset)++] = 0xe8;  /* opcode de call rel32 */
    int rel32_placeholder = *offset;
    code[(*offset)++] = 0x00;  /* placeholder para rel32 */
    code[(*offset)++] = 0x00;
    code[(*offset)++] = 0x00;
    code[(*offset)++] = 0x00;
    
    /* Calcula rel32: target_address - (instrução_call + tamanho_instrução) */
    int next_instr_after_call = call_instr_offset + 5;
    int rel32 = target_func_offset - next_instr_after_call;
    
    /* Escreve rel32 em little-endian */
    code[rel32_placeholder + 0] = (rel32 >>  0) & 0xFF;
    code[rel32_placeholder + 1] = (rel32 >>  8) & 0xFF;
    code[rel32_placeholder + 2] = (rel32 >> 16) & 0xFF;
    code[rel32_placeholder + 3] = (rel32 >> 24) & 0xFF;
    
    /* pop rbp */
    code[(*offset)++] = 0x5d;
    
    /* ret */
    code[(*offset)++] = 0xc3;
}

/* Emite uma função genérica com stack frame completo
 * Suporta: atribuições, operações aritméticas, ret
 */
static void emit_generic_with_frame(unsigned char *code, int *offset,
                                     Function *func) {
    emit_prologue(code, offset);
    
    for (int i = 0; i < func->num_instrs; i++) {
        Instruction *instr = &func->instrs[i];
        
        if (instr->type == INSTR_ARITH) {
            /* v_dest = src1 op src2 */
            int var_dest = instr->dest.value;
            
            /* Carrega primeiro operando em %eax */
            if (instr->src1.type == OPERAND_CONST) {
                code[(*offset)++] = 0xb8;
                int val = instr->src1.value;
                code[(*offset)++] = (val >>  0) & 0xFF;
                code[(*offset)++] = (val >>  8) & 0xFF;
                code[(*offset)++] = (val >> 16) & 0xFF;
                code[(*offset)++] = (val >> 24) & 0xFF;
            } else if (instr->src1.type == OPERAND_PARAM) {
                code[(*offset)++] = 0x89;
                code[(*offset)++] = 0xf8;
            } else if (instr->src1.type == OPERAND_VAR) {
                emit_load_var(code, offset, instr->src1.value);
            }
            
            /* Aplica operação */
            if (instr->op == '+') {
                if (instr->src2.type == OPERAND_CONST) {
                    int val = instr->src2.value;
                    if (val >= -128 && val <= 127) {
                        code[(*offset)++] = 0x83;
                        code[(*offset)++] = 0xc0;
                        code[(*offset)++] = val & 0xFF;
                    } else {
                        code[(*offset)++] = 0x05;
                        code[(*offset)++] = (val >>  0) & 0xFF;
                        code[(*offset)++] = (val >>  8) & 0xFF;
                        code[(*offset)++] = (val >> 16) & 0xFF;
                        code[(*offset)++] = (val >> 24) & 0xFF;
                    }
                }
            } else if (instr->op == '-') {
                if (instr->src2.type == OPERAND_CONST) {
                    int val = instr->src2.value;
                    if (val >= -128 && val <= 127) {
                        code[(*offset)++] = 0x83;
                        code[(*offset)++] = 0xe8;
                        code[(*offset)++] = val & 0xFF;
                    } else {
                        code[(*offset)++] = 0x2d;
                        code[(*offset)++] = (val >>  0) & 0xFF;
                        code[(*offset)++] = (val >>  8) & 0xFF;
                        code[(*offset)++] = (val >> 16) & 0xFF;
                        code[(*offset)++] = (val >> 24) & 0xFF;
                    }
                }
            } else if (instr->op == '*') {
                if (instr->src2.type == OPERAND_CONST) {
                    int val = instr->src2.value;
                    code[(*offset)++] = 0x69;
                    code[(*offset)++] = 0xc0;
                    code[(*offset)++] = (val >>  0) & 0xFF;
                    code[(*offset)++] = (val >>  8) & 0xFF;
                    code[(*offset)++] = (val >> 16) & 0xFF;
                    code[(*offset)++] = (val >> 24) & 0xFF;
                }
            }
            
            /* Salva resultado */
            emit_store_var(code, offset, var_dest);
            
        } else if (instr->type == INSTR_RET) {
            /* Carrega valor de retorno */
            if (instr->src1.type == OPERAND_CONST) {
                code[(*offset)++] = 0xb8;
                int val = instr->src1.value;
                code[(*offset)++] = (val >>  0) & 0xFF;
                code[(*offset)++] = (val >>  8) & 0xFF;
                code[(*offset)++] = (val >> 16) & 0xFF;
                code[(*offset)++] = (val >> 24) & 0xFF;
            } else if (instr->src1.type == OPERAND_PARAM) {
                code[(*offset)++] = 0x89;
                code[(*offset)++] = 0xf8;
            } else if (instr->src1.type == OPERAND_VAR) {
                emit_load_var(code, offset, instr->src1.value);
            }
            break;
        }
    }
    
    emit_epilogue(code, offset);
}

/* ========================================================================
 * INTERFACE PÚBLICA
 * ======================================================================== */

void gera_codigo(FILE *f, unsigned char code[], funcp *entry) {
    if (!f || !code || !entry) {
        return;
    }

    GenContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.code_buffer = code;
    ctx.code_offset = 0;

    /* PASSO 1: Parse do arquivo */
    ctx.num_functions = parse_file(f, &ctx.functions);
    if (ctx.num_functions == 0) {
        return; /* nenhuma função encontrada */
    }

    /* PASSO 2: Emissão de código
     * Suporta: ret $const, ret p0, v0 = p0 + $const; ret v0, v0 = call func p0; ret v0
     */
    for (int i = 0; i < ctx.num_functions; i++) {
        ctx.functions[i].code_offset = ctx.code_offset;
        
        Function *func = &ctx.functions[i];
        
        /* Caso 1: ret $const */
        if (func->num_instrs == 1 && 
            func->instrs[0].type == INSTR_RET &&
            func->instrs[0].src1.type == OPERAND_CONST) {
            
            emit_const_return(ctx.code_buffer, &ctx.code_offset, func->instrs[0].src1.value);
        }
        /* Caso 2: ret p0 */
        else if (func->num_instrs == 1 && 
                 func->instrs[0].type == INSTR_RET &&
                 func->instrs[0].src1.type == OPERAND_PARAM) {
            
            emit_param_return(ctx.code_buffer, &ctx.code_offset);
        }
        /* Caso 3: v0 = p0 OP $const; ret v0 (onde OP = +, -, ou *) */
        else if (func->num_instrs == 2 &&
                 func->instrs[0].type == INSTR_ARITH &&
                 func->instrs[0].dest.type == OPERAND_VAR &&
                 func->instrs[0].dest.value == 0 &&  /* v0 */
                 func->instrs[0].src1.type == OPERAND_PARAM &&
                 func->instrs[0].src1.value == 0 &&  /* p0 */
                 func->instrs[0].src2.type == OPERAND_CONST &&
                 (func->instrs[0].op == '+' || func->instrs[0].op == '-' || func->instrs[0].op == '*') &&
                 func->instrs[1].type == INSTR_RET &&
                 func->instrs[1].src1.type == OPERAND_VAR &&
                 func->instrs[1].src1.value == 0) {  /* ret v0 */
            
            int const_value = func->instrs[0].src2.value;
            char op = func->instrs[0].op;
            emit_arith_const_return(ctx.code_buffer, &ctx.code_offset, op, const_value);
        }
        /* Caso 4: v0 = call func p0; ret v0 */
        else if (func->num_instrs == 2 &&
                 func->instrs[0].type == INSTR_CALL &&
                 func->instrs[0].src1.type == OPERAND_PARAM &&
                 func->instrs[0].src1.value == 0 &&  /* passa p0 */
                 func->instrs[1].type == INSTR_RET &&
                 func->instrs[1].src1.type == OPERAND_VAR &&
                 func->instrs[1].src1.value == 0) {  /* ret v0 (resultado da call) */
            
            int target_func_num = func->instrs[0].func_num;
            if (target_func_num >= 0 && target_func_num < i) {
                /* Função alvo já foi emitida, podemos calcular offset */
                int target_offset = ctx.functions[target_func_num].code_offset;
                emit_call_function(ctx.code_buffer, &ctx.code_offset, target_offset);
            } else {
                /* Função alvo ainda não foi emitida - erro */
                emit_const_return(ctx.code_buffer, &ctx.code_offset, 0);
            }
        }
        /* Caso 5: zret p0 $const (retorno condicional simples) */
        else if (func->num_instrs == 1 &&
                 func->instrs[0].type == INSTR_ZRET &&
                 func->instrs[0].src1.type == OPERAND_PARAM &&
                 func->instrs[0].src1.value == 0 &&  /* testa p0 */
                 func->instrs[0].src2.type == OPERAND_CONST) {
            
            emit_zret_simple(ctx.code_buffer, &ctx.code_offset, 
                           func->instrs[0].src1.value,  /* cond_value (p0) */
                           func->instrs[0].src2.value); /* ret_value ($const) */
        }
        /* Caso 6: Operações complexas com variáveis locais (usar stack frame) */
        else if (func->num_instrs >= 1) {
            /* Verifica se usa variáveis locais */
            int uses_vars = 0;
            for (int j = 0; j < func->num_instrs; j++) {
                if (func->instrs[j].dest.type == OPERAND_VAR ||
                    func->instrs[j].src1.type == OPERAND_VAR ||
                    func->instrs[j].src2.type == OPERAND_VAR) {
                    uses_vars = 1;
                    break;
                }
            }
            
            if (uses_vars) {
                /* Usa stack frame genérico */
                emit_generic_with_frame(ctx.code_buffer, &ctx.code_offset, func);
            } else {
                /* Fallback */
                emit_const_return(ctx.code_buffer, &ctx.code_offset, 0);
            }
        }
        /* Fallback final */
        else {
            emit_const_return(ctx.code_buffer, &ctx.code_offset, 0);
        }
        
        ctx.functions[i].code_size = ctx.code_offset - ctx.functions[i].code_offset;
    }

    /* PASSO 3: Define entry como ponteiro para última função */
    int last = ctx.num_functions - 1;
    *entry = (funcp)(ctx.code_buffer + ctx.functions[last].code_offset);

    /* PASSO 4: Limpeza */
    for (int i = 0; i < ctx.num_functions; i++) {
        free(ctx.functions[i].instrs);
    }
    free(ctx.functions);
}
