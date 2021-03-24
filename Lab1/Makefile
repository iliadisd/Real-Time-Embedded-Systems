CC=gcc
CFLAGS=-O3

default: all

pc:
	$(CC) $(CFLAGS) -o pc prod-cons_8811.c -lpthread -lm

.PHONY: clean

all: pc
clean:
	rm -f pc
