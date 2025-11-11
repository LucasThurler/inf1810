# Código malicioso para passar argumento 0x01020304 para fizz
# O primeiro argumento em x86-64 é passado em %edi (ou %rdi para 64-bit)

movl $0x01020304, %edi
ret
