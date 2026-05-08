CC = gcc
CFLAGS = -Wall

all: rush

rush: rush.c
	$(CC) $(CFLAGS) rush.c -o rush

clean:
	rm -f rush