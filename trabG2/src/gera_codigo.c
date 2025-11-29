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
 * UTILITÁRIOS DE PARSING
 * ======================================================================== */

/* Pula espaços em branco e comentários */
static void skip_whitespace(FILE *f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (isspace(c)) continue;
        if (c == ';') {
            /* comentário: pula até fim de linha */
            while ((c = fgetc(f)) != EOF && c != '\n');
            continue;
        }
        ungetc(c, f);
        break;
    }
}

/* Lê um identificador/palavra-chave */
static int read_identifier(FILE *f, char *buf, int max_len) {
    skip_whitespace(f);
    int i = 0;
    int c;
    while ((c = fgetc(f)) != EOF && (isalnum(c) || c == '_')) {
        if (i < max_len - 1) buf[i++] = c;
    }
    if (c != EOF) ungetc(c, f);
    buf[i] = '\0';
    return i > 0;
}

/* Lê um número inteiro (com sinal opcional) */
static int read_number(FILE *f, int *out) {
    skip_whitespace(f);
    int c;
    int sign = 1;
    int num = 0;
    int found_digit = 0;

    c = fgetc(f);
    if (c == '-') {
        sign = -1;
        c = fgetc(f);
    } else if (c == '+') {
        c = fgetc(f);
    }

    while (c != EOF && isdigit(c)) {
        num = num * 10 + (c - '0');
        found_digit = 1;
        c = fgetc(f);
    }
    if (c != EOF) ungetc(c, f);

    if (found_digit) {
        *out = sign * num;
        return 1;
    }
    return 0;
}

/* Lê um operando (const $n, var v0..v4, ou param p0) */
static Operand read_operand(FILE *f) {
    Operand op;
    op.type = OPERAND_INVALID;
    op.value = 0;

    skip_whitespace(f);
    int c = fgetc(f);

    if (c == '$') {
        /* constante */
        int num;
        if (read_number(f, &num)) {
            op.type = OPERAND_CONST;
            op.value = num;
        }
    } else if (c == 'v') {
        /* variável local */
        int idx;
        if (read_number(f, &idx) && idx >= 0 && idx <= 4) {
            op.type = OPERAND_VAR;
            op.value = idx;
        }
    } else if (c == 'p') {
        /* parâmetro p0 */
        int idx;
        if (read_number(f, &idx) && idx == 0) {
            op.type = OPERAND_PARAM;
            op.value = 0;
        }
    } else {
        if (c != EOF) ungetc(c, f);
    }

    return op;
}

/* ========================================================================
 * PARSER
 * ======================================================================== */

/* Faz parse de uma instrução dentro de uma função
 * Retorna tipo de instrução ou INSTR_INVALID se falhar
 */
static Instruction parse_instruction(FILE *f) {
    Instruction instr;
    memset(&instr, 0, sizeof(instr));
    instr.type = INSTR_INVALID;

    char keyword[64];
    if (!read_identifier(f, keyword, sizeof(keyword))) {
        return instr;
    }

    /* Tenta interpretar como instrução */
    if (strcmp(keyword, "ret") == 0) {
        /* ret expr */
        instr.type = INSTR_RET;
        instr.src1 = read_operand(f);
        if (instr.src1.type == OPERAND_INVALID) {
            instr.type = INSTR_INVALID;
        }
    } else if (strcmp(keyword, "zret") == 0) {
        /* zret cond expr */
        instr.type = INSTR_ZRET;
        instr.src1 = read_operand(f);
        instr.src2 = read_operand(f);
        if (instr.src1.type == OPERAND_INVALID || instr.src2.type == OPERAND_INVALID) {
            instr.type = INSTR_INVALID;
        }
    } else if (strcmp(keyword, "call") == 0) {
        /* call func_num arg */
        if (!read_number(f, &instr.func_num)) {
            return instr;
        }
        instr.type = INSTR_CALL;
        instr.src1 = read_operand(f);
        if (instr.src1.type == OPERAND_INVALID) {
            instr.type = INSTR_INVALID;
        }
    } else {
        /* Assumir que é uma atribuição: var = expr */
        /* keyword deve ser um v0..v4 ou p0 */
        if (keyword[0] == 'v' && strlen(keyword) == 2 &&
            isdigit(keyword[1])) {
            int idx = keyword[1] - '0';
            if (idx >= 0 && idx <= 4) {
                instr.dest.type = OPERAND_VAR;
                instr.dest.value = idx;

                skip_whitespace(f);
                int c = fgetc(f);
                if (c == '=') {
                    /* lê operandos e operador aritmético */
                    instr.src1 = read_operand(f);
                    if (instr.src1.type == OPERAND_INVALID) {
                        return instr;
                    }

                    skip_whitespace(f);
                    c = fgetc(f);
                    if (c == '+' || c == '-' || c == '*') {
                        instr.op = c;
                        instr.type = INSTR_ARITH;
                        instr.src2 = read_operand(f);
                        if (instr.src2.type == OPERAND_INVALID) {
                            instr.type = INSTR_INVALID;
                        }
                    } else {
                        /* sem operador: apenas atribuição */
                        if (c != EOF) ungetc(c, f);
                        instr.type = INSTR_ASSIGN;
                        instr.src1 = read_operand(f);
                    }
                } else {
                    if (c != EOF) ungetc(c, f);
                }
            }
        }
    }

    return instr;
}

/* Faz parse de uma função completa (function ... end) */
static Function parse_function(FILE *f, int func_num) {
    Function func;
    memset(&func, 0, sizeof(func));
    func.num = func_num;

    /* Aloca array inicial de instruções */
    int capacity = 16;
    func.instrs = malloc(capacity * sizeof(Instruction));
    func.num_instrs = 0;

    char keyword[64];
    while (read_identifier(f, keyword, sizeof(keyword))) {
        if (strcmp(keyword, "end") == 0) {
            break;
        }

        /* Volta para reler como instrução */
        ungetc(keyword[0], f);
        fseek(f, -strlen(keyword), SEEK_CUR);

        Instruction instr = parse_instruction(f);
        if (instr.type != INSTR_INVALID) {
            if (func.num_instrs >= capacity) {
                capacity *= 2;
                func.instrs = realloc(func.instrs, capacity * sizeof(Instruction));
            }
            func.instrs[func.num_instrs++] = instr;
        }
    }

    return func;
}

/* Faz parse do arquivo LBS completo */
static int parse_file(FILE *f, Function **out_funcs) {
    int num_functions = 0;
    int capacity = 8;
    *out_funcs = malloc(capacity * sizeof(Function));

    char keyword[64];
    while (read_identifier(f, keyword, sizeof(keyword))) {
        if (strcmp(keyword, "function") == 0) {
            if (num_functions >= capacity) {
                capacity *= 2;
                *out_funcs = realloc(*out_funcs, capacity * sizeof(Function));
            }
            (*out_funcs)[num_functions] = parse_function(f, num_functions);
            num_functions++;
        }
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

/* Emite uma função com operação aritmética simples: p0 + $const
 * Sem prologue/epilogue (funciona para x86-64 System V ABI sem variáveis locais).
 * 
 * Bytecode baseado em assembly real:
 *   89 f8        mov %edi, %eax       (p0 para eax)
 *   83 c0 XX     add $XX, %eax        (add constante)
 *   c3           ret
 */
static void emit_add_const_return(unsigned char *code, int *offset, int constant) {
    /* mov %edi, %eax (p0 para eax) */
    code[(*offset)++] = 0x89;
    code[(*offset)++] = 0xf8;
    
    /* add $constant, %eax */
    if (constant >= -128 && constant <= 127) {
        /* add $imm8, %eax (instrução curta) */
        code[(*offset)++] = 0x83;
        code[(*offset)++] = 0xc0;
        code[(*offset)++] = constant & 0xFF;
    } else {
        /* add $imm32, %eax (instrução longa) */
        code[(*offset)++] = 0x05;
        code[(*offset)++] = (constant >>  0) & 0xFF;
        code[(*offset)++] = (constant >>  8) & 0xFF;
        code[(*offset)++] = (constant >> 16) & 0xFF;
        code[(*offset)++] = (constant >> 24) & 0xFF;
    }
    
    /* ret */
    code[(*offset)++] = 0xc3;
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
        /* Caso 3: v0 = p0 + $const; ret v0 */
        else if (func->num_instrs == 2 &&
                 func->instrs[0].type == INSTR_ARITH &&
                 func->instrs[0].dest.type == OPERAND_VAR &&
                 func->instrs[0].dest.value == 0 &&  /* v0 */
                 func->instrs[0].src1.type == OPERAND_PARAM &&
                 func->instrs[0].src1.value == 0 &&  /* p0 */
                 func->instrs[0].src2.type == OPERAND_CONST &&
                 func->instrs[0].op == '+' &&
                 func->instrs[1].type == INSTR_RET &&
                 func->instrs[1].src1.type == OPERAND_VAR &&
                 func->instrs[1].src1.value == 0) {  /* ret v0 */
            
            int const_value = func->instrs[0].src2.value;
            emit_add_const_return(ctx.code_buffer, &ctx.code_offset, const_value);
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
        /* Fallback */
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
