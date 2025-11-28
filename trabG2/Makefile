# Makefile para gerador de código LBS
# Compilação com gcc, flags para permitir execução de código no stack

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC -z execstack
LDFLAGS = -z execstack

# Diretórios
SRC = src
INCLUDE = include
TESTS = tests
EXAMPLES = examples
BUILD = build

# Arquivos fonte
GERA_CODIGO_SRC = $(SRC)/gera_codigo.c
TEST_SRC = $(TESTS)/test_basic.c

# Targets
GERA_CODIGO_OBJ = $(BUILD)/gera_codigo.o
TEST_BIN = $(BUILD)/test_basic

# Targets principais
.PHONY: all build test clean

all: build test

build: $(TEST_BIN)

test: $(TEST_BIN)
	@echo "=== Rodando testes de sanidade ==="
	@$(TEST_BIN) $(EXAMPLES)/simple.lbs
	@echo "=== Sucesso! ==="

clean:
	rm -rf $(BUILD)

# Regras de compilação
$(BUILD):
	mkdir -p $(BUILD)

$(GERA_CODIGO_OBJ): $(GERA_CODIGO_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $(GERA_CODIGO_SRC) -o $(GERA_CODIGO_OBJ)

$(TEST_BIN): $(GERA_CODIGO_OBJ) $(TEST_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(GERA_CODIGO_OBJ) $(TEST_SRC) -o $(TEST_BIN) $(LDFLAGS)

.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: clean build
	@echo "Build com debug ativado"

.PHONY: help
help:
	@echo "Targets disponíveis:"
	@echo "  make build  - Compilar teste"
	@echo "  make test   - Executar testes de sanidade"
	@echo "  make clean  - Limpar arquivos gerados"
	@echo "  make debug  - Compilar com símbolos de debug"
	@echo "  make help   - Mostrar esta mensagem"
