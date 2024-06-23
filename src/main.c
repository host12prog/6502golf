#define VERSION 3
#define G_STDOUT 0xFFE0
#define G_STDIN 0xFFE1
#define G_COUNT 0xFFE2

#include "fake6502.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
// TO-DO: termios is incompatible with windows
//        and i should find something else

uint8_t memory[0xFFFF] = {0};

uint8_t fake6502_mem_read(fake6502_context *c, uint16_t addr) {
  int cb;
  switch (addr) {
    case G_STDIN:
      cb = getchar();
      if (cb != EOF) {
        memory[G_STDIN] = cb;
      } else {
        memory[G_STDIN] = 0xFF;
      }
      // side-effect: 0xFFE1 is read-only(?)
      return memory[addr];
    default:
      return memory[addr];
  }
}

void fake6502_mem_write(fake6502_context *c, uint16_t addr, uint8_t val) {
  switch (addr) {
    case G_STDOUT:
      putchar(val);
      // side-effect: 0xFFE0 is write-only
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
                  "  -h --help:            show this help message\n"
                  "  -x --no-load-address: ignore load address header of binary\n"
                  "                        and load at $0000\n");
}

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
  // dilemma: set pc or set reset vector?
  c->cpu.pc = 0x0200;
}

int main(int argc, char *argv[]) {
  printf("6502GOLF version %d (-h or --help for options)\n", VERSION);
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
  static struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  fake6502_context f6502; // init cpu
  init(&f6502);
  clock_t start_time = clock();
  int exec_code;
  //uint16_t timer_val;
  for (;;) {
    //timer_val = (memory[0xFFE2] << 8) | memory[0xFFE1];
    exec_code = execute(&f6502);
    if (exec_code == 1) break;
    if (clock() >= start_time + 20) {
      fake6502_irq(&f6502);
      start_time = clock();
      memory[G_COUNT]++;
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}
