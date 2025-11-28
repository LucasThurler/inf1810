# Gerador de Código LBS → x86-64

Esqueleto para gerador de código da disciplina INF1018 (2025.2).

## Estrutura do Repositório

```
trabG2/
├── include/
│   └── gera_codigo.h          # Interface pública (NÃO MODIFICAR)
├── src/
│   └── gera_codigo.c          # Implementação do gerador
├── tests/
│   └── test_basic.c           # Testes de sanidade
├── examples/
│   └── simple.lbs             # Exemplos LBS para validar parser/emitter
├── Makefile                   # Build system
└── README.md                  # Este arquivo
```

## Compilação e Testes

```bash
make build    # Compilar apenas
make test     # Compilar e rodar testes
make clean    # Limpar artefatos
make debug    # Build com símbolos de debug (-g)
make help     # Mostrar targets disponíveis
```

## Estado Atual (Commit 0)

✅ **Estrutura base criada e funcionando:**
- Parser minimalista que lê funções LBS
- Estruturas de dados intermediárias (Function, Instruction, Operand)
- Teste que compila, aloca buffer executável, e chama função gerada
- Emitter stub que gera `xor eax, eax; ret` (retorna 0)

⚠️ **O que falta (próximos commits):**

### Commit 1: Parser Completo
- [ ] Validação robusta de sintaxe LBS
- [ ] Mensagens de erro descritivas com linha/coluna
- [ ] Suporte completo para todas as formas de operandos

### Commit 2: Emitter Básico (ret $const)
- [ ] Gerar `mov eax, imm32; ret` para retornar constantes
- [ ] Gerir offsets de função e reloacação de endereços
- [ ] Testar com função que retorna $100, $-42, etc.

### Commit 3: Emitter Aritmética (v0 = p0 + $1)
- [ ] Alocação de variáveis locais no stack ([rbp-8], [rbp-16], ...)
- [ ] Prologue/epilogue (push rbp; mov rbp, rsp; sub rsp, 32 etc)
- [ ] Carregar parâmetro de rdi para register/stack
- [ ] Instruções add/sub/imul x86-64
- [ ] Testes com v0 = p0 + const, v0 = v1 + v2, etc.

### Commit 4: Chamadas de Função (call)
- [ ] Salvamento/restauração de registradores caller-saved
- [ ] Cálculo de offsets para chamadas relativas (call rel32)
- [ ] Passar argumento em rdi (first argument)
- [ ] Testar funções que chamam funções

### Commit 5: Retorno Condicional (zret)
- [ ] Comparação (cmp) e salto condicional (je)
- [ ] Variação entre retorno incondicional e condicional

## Notas de Implementação

### Parser
- Usa funções simples `read_identifier()`, `read_number()`, `read_operand()`
- Para cada função, cria array de instruções intermediárias
- Operandos classificados em 3 tipos: OPERAND_CONST, OPERAND_VAR, OPERAND_PARAM

### Emitter (stub → real)
- Buffer `code[]` é alocado com `posix_memalign()` e marcado com `mprotect(..., PROT_EXEC)`
- Cada função tem `code_offset` (posição no buffer) e `code_size` (bytes emitidos)
- *entry aponta para o início da última função

### Convenção de Chamada (System V AMD64 ABI)
- Parâmetro inteiro em `rdi` (primeiros 32 bits = `edi`)
- Retorno em `rax` (primeiros 32 bits = `eax`)
- Locais salvos em stack: `[rbp-8]` = v0, `[rbp-16]` = v1, etc.
- Caller-saved: rax, rcx, rdx, rsi, rdi, r8-r11

## Exemplo de Uso

```bash
$ cat examples/simple.lbs
function
  ret $100
end

$ ./build/test_basic examples/simple.lbs
Código gerado: 3 bytes
Entry point: 0x...
=== Teste 1: Função que retorna constante $100 ===
entry(42) = 0 (esperado: 0 no stub)
=== Todos os testes passaram! ===
```

## Próximos Passos

1. Implementar parsing robusto com tratamento de erros
2. Emitter básico: `mov eax, imm32; ret`
3. Stack frame e prologue/epilogue
4. Instruções aritméticas
5. Chamadas de função e reloacação
6. Retorno condicional

---

**Autores:** INF1018 (2025.2)  
**Data:** 2025-11-28
