CC=gcc
CFLAGS=-I. -Wall
DEPS=fake6502.h

%.o: src/%.c src/$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: fake6502.o main.o
	$(CC) -o 6502golf fake6502.o main.o
