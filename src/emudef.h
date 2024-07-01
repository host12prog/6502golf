#ifndef EMUDEF_H
#define EMUDEF_H
/* build info */
#define CMOS6502 1
#define DECIMALMODE 1
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
#endif
