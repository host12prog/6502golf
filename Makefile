CC=gcc
CPUTYPE = CMOS6502
CFLAGS=-I. -Wall
DEPS = src/mmu.h src/fake6502.h
OBJ = main.o mmu.o fake6502.o

%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

6502golf: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
