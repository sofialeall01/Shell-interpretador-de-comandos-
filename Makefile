# define o compilador
CC = gcc

# ativa avisos de erros
CFLAGS = -Wall

# codigo p compilar tudo
all: rush

# cria um executavel rush qa partir do codigo rush.c
rush: rush.c
	$(CC) $(CFLAGS) rush.c -o rush

# limpeza
clean:
	rm -f rush
