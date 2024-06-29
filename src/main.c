#define NMOS_6502 1
#define DECIMAL_MODE 1
#define VERSION 4
#define SUBVERSION 1
#define G_STDOUT 0xFFE0
#define G_STDIN 0xFFE1
#define G_COUNT 0xFFE2
#define G_IRQACK 0xFFE3
#define G_STDERR 0xFFE4

#include "fake6502.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
// TO-DO: termios is incompatible with windows
//				and i should find something else

uint8_t memory[0xFFFF] = {0};
struct termios orig_termios;

void disableRawMode() {
	if (tcsetattr(0, TCSAFLUSH, &orig_termios) == -1)
		exit(2);
}
void sigintdrm(int a) { (void)a; disableRawMode(); exit(0); }

void enableRawMode() {
	if (tcgetattr(0, &orig_termios) == -1) exit(2);
	signal(SIGINT, sigintdrm);
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSAFLUSH, &raw) == -1)
		exit(2);
}

uint8_t fake6502_mem_read(fake6502_context *c, uint16_t addr) {
	return memory[addr];
}

void fake6502_mem_write(fake6502_context *c, uint16_t addr, uint8_t val) {
	switch (addr) {
		case G_STDOUT:
			putchar(val);
			// side-effect: 0xFFE0 is write-only
			break;
		case G_STDERR:
			fprintf(stderr, "%c", val);
			break;
		default:
			memory[addr] = val;
			break;
	}
}

int load(const char* fn, int load_addr_ign) {
	FILE *f = fopen(fn, "r");
	if (f == NULL) return -1;
	uint8_t lb, hb;
	int c;
	fseek(f, 0, SEEK_SET);
	hb = fgetc(f);
	lb = fgetc(f);
	uint16_t load_addr;
	if (load_addr_ign == 0) load_addr = ((lb & 0xFF) << 8) | (hb & 0xFF);
	else load_addr = 0;
	fseek(f, 0, 1);
	for (int i=0;(c=fgetc(f))!=EOF;i++) {
		memory[load_addr+i] = c;
	}
	fclose(f);
	return load_addr;
}

void usage() {
	fprintf(stdout, "usage: 6502golf [options] program-binary\n"
									"options:\n"
									"	-h --help:						show this help message\n"
									"	-x --no-load-address: ignore load address header of binary\n"
									"												and load at $0000\n");
}

int execute(fake6502_context *c) {
	uint16_t lastpc;
	lastpc = c->cpu.pc; // hack: if BRK is executed the PC is 0000
	fake6502_step(c);
	if (c->emu.opcode == 0x00) {
		printf("\r\n\r\n(BRK encountered) A: $%02X, X: $%02X, Y: $%02X, SP: $%02X, PC: $%04X, Flags: $%02X\r\n", c->cpu.a, c->cpu.x, c->cpu.y, c->cpu.s, lastpc, c->cpu.flags);
		return 1;
	}
	return 0;
}

void init(fake6502_context *c) {
	fake6502_reset(c); // init cpu
	// to-do: why have a function for this?
}

int main(int argc, char *argv[]) {
	printf("6502GOLF version %d.%d (-h or --help for options)\n", VERSION, SUBVERSION);
	printf("6502GOLF comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome to redistribute it under certain conditions; See LICENSE!\n\n");
	int load_addr_ignore = 0;
	if (argc > 1) {
		// ach, if-else ladders
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			usage();
		} else if (strcmp(argv[1], "-x") == 0 || strcmp(argv[1], "--no-load-address") == 0) {
			load_addr_ignore = 1;
		}
		int load_addr = load(((load_addr_ignore == 1) ? argv[2] : argv[1]), load_addr_ignore);
		if (load_addr == -1) {
			fprintf(stderr, "File does not exist!\n");
			return 1;
		}
		printf("Program loaded at $%04X", load_addr);
	} else {
		printf("No binary found! (Version 3 onwards do not include a test ROM)\n");
		return 1;
	}
	// argc argv cruft
	// init terminal
	enableRawMode();
	fake6502_context f6502; // init cpu
	init(&f6502);
	clock_t start_time = clock();
	clock_t diff;
	int exec_code;
	int cb;
	//uint16_t timer_val;
	for (;;) {
		//timer_val = (memory[0xFFE2] << 8) | memory[0xFFE1];
		exec_code = execute(&f6502);
		cb = getchar();
		if (exec_code == 1) break;
		if (cb == 3) break; // ctrl+c handle
		if (cb != EOF)
			memory[G_STDIN] = cb;
		else
			memory[G_STDIN] = 0;
		if (((diff * 1000 / CLOCKS_PER_SEC) % 1000) == 20) {
			//printf("%02x ", memory[G_IRQACK]);
			//printf("%02x ", (memory[G_IRQACK] & 0xFE));
			if ((memory[G_IRQACK] & 0xFE) == 0) {
				memory[G_IRQACK] |= 0x01;
				//printf("%02x ", memory[G_IRQACK]);
				fake6502_irq(&f6502);
				start_time = clock();
			}
			memory[G_COUNT]++;
		}
		diff = clock() - start_time;
	}
	disableRawMode();
	return 0;
}
