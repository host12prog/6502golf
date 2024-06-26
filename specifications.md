# 6502golf

6502golf is a code golf machine which is currently suffering from feature creep!

- WDC 65C02 (8.0 MHz, 50hz VBLANK)
- 64K of RAM
- dumb terminal display

## BOOT-UP

Before RESET, the memory is loaded with machine code (written by the user). On boot-up, the CPU jumps to the RESET vector specified at vector `$FFFC`-`$FFFD`.

Before version 3, the machine would force the program counter of the CPU to `$0200`. The stack pointer is set to $FD on boot-up, so if you wish to use all of the stack, store an $FF into the stack pointer: 
```
  LDX #$FF
  TXS
```

## EXECUTION

The CPU will begin execution at the RESET vector specified by the program. Here, you can jump to somewhere else to begin program execution. It is not recommended that you try to write a program within the RESET handler. An RTI instruction is not needed, as RESET only occurs once on boot-up.

## INTERRUPTS

Every 20ms (1/50th of a second), a VBLANK IRQ will occur, however what this IRQ does is left up to the user. A design choice was made so that it generated IRQs instead of NMIs, so if the user wishes to not use these interrupts, they can add a SEI instruction to their program. A consequence is that the NMI line has been left unused, so an NMI can never occur in normal operation.

It is inaccurate to call these IRQs VBLANK IRQs, as the display is a dumb terminal and as such does not have a VBLANK, however internally the machine keeps track of how many cycles left until VBLANK. The machine will run for 160,000 cycles, and when it is done, it will wait for +/-20ms for the next frame

Whenever these IRQs occur, a counter is also incremented by 1. This counter is located at memory location $FFE2, and can be read from or written to. When this interrupt is triggered, it sets bit 0 of `$FFE3` (IRQACK). The program must acknowledge the IRQ by clearing bit 0 of IRQACK. The most orthodox way of doing this:
```
  LDA #0 ; %00000000
         ;         ^-- VBLANK IRQACK
  STA $FFE3 ; Acknowledge the IRQ request
```

The remaining bits in IRQACK are unused, and if read, will be clear.

## DISPLAY

The display is a dumb terminal display, emulated by your terminal emulator. Because it is a dumb terminal, one can send control codes and ANSI escape sequences (to set cursor, color, etc...).

Only one byte can be output to the display at a time. The standard output is located at memory location `$FFE0`. Bytes written here must be valid ASCII. Alternatively, the program can write to `$FFE4`, which will send it to standard error (STDERR). To allow for user input, standard input also exists, and is located at memory location `$FFE1`, which is a read-only memory location, so any writes here will fail.

Whenever a key is pressed, the key's ASCII value is sent to STDIN, however the value will persist in STDIN for some time, and to prevent duplication when printing STDIN to STDOUT/STDERR, the program must enter an infinite loop to wait for STDIN to be zero again:
```
+ LDA $FFE1 ; Read STDIN
  BNE + ; If it is not zero, jump back to
        ; reading STDIN until it's zero
```

Note that STDOUT (standard output, $FFE0) is a write-only memory location. If a read is attempted on this address, it will be a value of `$00`.

If one wishes to display more complex graphics, they can utilize ANSI escape sequences. To help with this task, the program can read TERMROW (`$FFE5`) and TERMCOL (`$FFE6`) to get the terminal  height and width. TERMROW and TERMCOL are 10-bit values, meaning the locations described above only show the lower 8 bits of the 10-bit value. The remaining 2 bits are collected at TERMHIGH (`$FFE7`):
```
  ----xxyy
xx: topmost 2 bits of TERMROW
yy: topmost 2 bits of TERMCOL
```

## USEFUL MEMORY LOCATIONS

- `$0000-$00FF`: Zero-Page (256 bytes)
- `$0100-$01FF`: Stack (256 bytes)
- `$0200-$FEFF`: General-Purpose RAM (64767 bytes)
- `$FF00-$FFDF`: Reserved I/O (223 bytes)
- `$FFE0`: STDOUT (Standard Output)
- `$FFE1`: STDIN (Standard Input)
- `$FFE2`: COUNT (50hz Counter)
- `$FFE3`: IRQACK (Interrupt Acknowledge)
- `$FFE4`: STDERR (Standard Error)
- `$FFE5`: TERMROW (Terminal Rows, 10-bit)
- `$FFE6`: TERMCOL (Terminal Columns, 10-bit)
- `$FFE7`: TERMHIGH (remaining 2 bits of TERMROW and TERMCOL)
- `$FFE8-$FFF9`: Reserved (17 bytes)
- `$FFFA-$FFFB`: NMI vector (Unused)
- `$FFFC-$FFFD`: RESET vector (Boot-Up)
- `$FFFE-$FFFF`: IRQ vector (50hz)

(Note: The 6502 is little-endian, so for the vectors, you must load the low byte first ($xxYY) and then load the high byte last ($XXyy). In assembler syntax, low byte would be: `#<label`, and high byte would be: `#>label`)
