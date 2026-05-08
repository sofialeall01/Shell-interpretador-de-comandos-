# variaveis
CC = gcc
CFLAGS = -Wall

# código p compilar tudo
all: rush

# cria um executavel rush que roda em cima do codigo rush.c
rush: rush.c
	$(CC) $(CFLAGS) rush.c -o rush

# limpeza
clean:
	rm -f rush
