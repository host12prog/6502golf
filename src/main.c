#define VERSION 1

#include "fake6502.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint8_t memory[0xFFFF] = {0};
static const uint8_t _6502golf_rom[]  = {
  0xb9, 0x0e, 0x02, 0xc9, 0x00, 0xf0, 0x06, 0x85, 0x00, 0xc8, 0x4c, 0x00, 0x02, 0x00, 0x36, 
  0x35, 0x30, 0x32, 0x47, 0x4f, 0x4c, 0x46, 0x20, 0x54, 0x45, 0x53, 0x54, 0x20, 0x52, 0x4f, 
  0x4d, 0x0a, 0x00
};
// yeah it's small enough to fit here
// got a problem?

uint8_t fake6502_mem_read(fake6502_context *c, uint16_t addr) {
  int cb;
  switch (addr) {
    case 0x01:
      cb = getchar();
      if (cb != EOF) {
        memory[0x01] = cb;
      } else {
        memory[0x01] = 0xFF;
      }
      // side-effect: 0x01 is read-only(?)
      return memory[addr];
    default:
      return memory[addr];
  }
}

void fake6502_mem_write(fake6502_context *c, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0x00:
      putchar(val);
      // side-effect: 0x00 is write-only
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

/*
void usage() {
  fprintf(stdout, "usage: 6502golf [options] program-binary\n"
                  "options:\n"
                  "  -x: ignore load address header of binary\n"
                  "      and load at $0000\n");
}
*/

int execute(fake6502_context *c) {
  uint16_t lastpc;
  lastpc = c->cpu.pc; // hack: if BRK is executed the PC is 0000
  fake6502_step(c);
  if (c->emu.opcode == 0x00) {
    printf("\n\n(BRK encountered) A: $%02X, X: $%02X, Y: $%02X, SP: $%02X, PC: $%04X, Flags: $%02X\n", c->cpu.a, c->cpu.x, c->cpu.y, c->cpu.s, lastpc, c->cpu.flags);
    return 1;
  }
  return 0;
}

void init(fake6502_context *c) {
  fake6502_reset(c); // init cpu
  c->cpu.pc = 0x0200;
}

int main(int argc, char *argv[]) {
  printf("6502GOLF version %d\n", VERSION);
  printf("6502GOLF comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome to redistribute it under certain conditions; See LICENSE!\n\n");
  int load_addr_ignore = 0;
  if (argc > 1) {
    int load_addr = load(argv[1], load_addr_ignore);
    if (load_addr == -1) {
      fprintf(stderr, "File does not exist!\n");
      return 1;
    }
    printf("Program loaded at $%04X", load_addr);
  } else {
    for (int i=0;i<33;i++) {
      memory[0x200+i] = _6502golf_rom[i];
    }
  }
  fake6502_context f6502;
  init(&f6502);
  clock_t start_time = clock();
  int exec_code;
  for (;;) {
    exec_code = execute(&f6502);
    if (exec_code == 1) break;
    if (clock() >= start_time + 20) {
      fake6502_irq(&f6502);
      start_time = clock();
      memory[0xFFE0]++;
    }
  }
  return 0;
}
