#include "emudef.h"
#include "fake6502.h"
#include <stdint.h>
#include <stdio.h>

uint8_t memory[0xFFFF] = {0};

void mem_fwrite(uint16_t addr, uint8_t val) {
	memory[addr] = val;
}

uint8_t mem_read(uint16_t addr) {
	return memory[addr];
}

void mem_write(uint16_t addr, uint8_t val) {
	switch (addr) {
		case G_STDOUT:
			putchar(val);
			break;
		case G_STDERR:
			fprintf(stderr, "%c", val);
			break;
		// read-only cases
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

uint8_t fake6502_mem_read(fake6502_context *c, uint16_t addr) {
	return mem_read(addr);
}

void fake6502_mem_write(fake6502_context *c, uint16_t addr, uint8_t val) {
	mem_write(addr, val);
}
