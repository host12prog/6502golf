CC=gcc
CPUTYPE=NMOS6502
CFLAGS=-D $(CPUTYPE) -I. -Wall
DEPS = fake6502.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: fake6502.o main.o 
	$(CC) -o 6502golf fake6502.o main.o 