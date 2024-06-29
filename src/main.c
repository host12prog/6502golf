/* build info */
#define CMOS_6502 1
#define DECIMAL_MODE 1
#define VERSION 5
#define SUBVERSION 1
/* memory addresses */
#define G_IOBASE 0xFFE0
#define G_STDOUT (G_IOBASE)
#define G_STDIN (G_IOBASE+1)
#define G_COUNT (G_IOBASE+2)
#define G_IRQACK (G_IOBASE+3)
#define G_STDERR (G_IOBASE+4)
#define G_TERMROW (G_IOBASE+5)
#define G_TERMCOL (G_IOBASE+6)
#define G_TERMHIGH (G_IOBASE+7)
/* emulator clock */
#define G_CLOCK 8000000 // 8 MHz
#define G_FPS 50
#define G_CYCLES (G_CLOCK / G_FPS) // cycles per frame
#define G_TICK (CLOCKS_PER_SEC / G_FPS)
/* headers */
#include "fake6502.h"
#include "libt85apu/t85apu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
// TO-DO: termios is incompatible with windows
//				and i should find something else

uint8_t memory[0xFFFF] = {0};
struct termios orig_termios;
fake6502_context f6502; // init cpu

void disableRawMode() {
	if (tcsetattr(0, TCSAFLUSH, &orig_termios) == -1)
		exit(2);
}
void sigintdrm(int a) {
	(void)a;
	disableRawMode();
	printf("\n\nA: $%02X, X: $%02X, Y: $%02X, SP: $%02X, PC: $%04X, Flags: $%02X\n", f6502.cpu.a, f6502.cpu.x, f6502.cpu.y, f6502.cpu.s, f6502.cpu.pc, f6502.cpu.flags);
	exit(0);
}

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

void getTermXY() {
	struct winsize ws;
	uint16_t col;
	uint16_t row;
	int fd;
	fd = open("/dev/tty", O_RDWR);
	if (fd < 0) return;
	if (ioctl(fd, TIOCGWINSZ, &ws) < 0) return;
	col = ws.ws_col;
	row = ws.ws_row;
	memory[G_TERMCOL] = col & 0xFF;
	memory[G_TERMROW] = row & 0xFF;
	memory[G_TERMHIGH] = ((col & 0x300) >> 6) | ((row & 0x300) >> 8);
	close(fd);
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
		case G_STDIN:
		case G_TERMROW:
		case G_TERMCOL:
		case G_TERMHIGH:
			break;
		default:
			memory[addr] = val;
			break;
	}
}

int load(const char* fn, int load_addr_ign) {
  // i should use fwrite here
  // but this spaghetti code works fine enough
  // we only call it once anyways
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

void execute(fake6502_context *c) {
	c->emu.clockticks = 0;
	fake6502_step(c);
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
	getTermXY();
	enableRawMode();
	init(&f6502);
	int cb;
	for (;;) {
		clock_t it_time = clock() + G_TICK;
		for (int i=0;i<G_CYCLES;i+=f6502.emu.clockticks) {
			execute(&f6502);
		}
		memory[G_COUNT]++;
		cb = getchar();
		if (cb == 3) break; // ctrl+c handle
		if (cb != EOF)
			memory[G_STDIN] = cb;
		else
			memory[G_STDIN] = 0;
		if ((memory[G_IRQACK] & 0xFE) == 0) {
			memory[G_IRQACK] |= 0x01;
			fake6502_irq(&f6502);
		}
		for (;;) {
			if (clock() < it_time) continue;
			else break;
		}
	}
	disableRawMode();
	return 0;
}
