#ifndef MMU_H
#define MMU_H
#include "emudef.h"
#include "fake6502.h"
#include <stdint.h>
#include <stdio.h>

void mem_fwrite(uint16_t addr, uint8_t val);
uint8_t mem_read(uint16_t addr);
void mem_write(uint16_t addr, uint8_t val);
uint8_t fake6502_mem_read(fake6502_context *c, uint16_t addr);
void fake6502_mem_write(fake6502_context *c, uint16_t addr, uint8_t val);
#endif
