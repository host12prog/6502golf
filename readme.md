# 6502golf

6502 code golf machine (inspired by mokehehe's z80golf) Useful for getting to grips with 6502 first, before coding in 6502 for another system (like Commodore 64, VIC-20, NES, ...)

## BUILDING

Very unfortunately, I cannot provide binaries for you right now, but building 6502golf is easy and requires no other dependencies. All you need are:

- a C compiler (preferably gcc)
- make (to make the Makefile, duh)

To build 6502golf, simply type "make". (should work on all *NIX systems) For Windows, you could try building it using MinGW or cygwin. I haven't got it quite figured out for Windows, yet so if anyone has any more ideas, please send me a pull request.

## SPECIFICATIONS

- 6502golf supports CMOS 65C02 by default.
  - You can use CMOS 65C02 but you'll have to fiddle with the source to get it to build. Change `CMOS6502` to `NMOS6502` in `main.c`.
- Since version 3, a test ROM is no longer built into the program. Therefore, a binary must be provided to 6502golf.
- If a binary is provided, it will be loaded at the load address specified in the first 2 bytes of the binary (unless -x or --no-load-address is specified)
- For the memory map and operation, read the included [specifications.md](specifications.md) file.

## CREDITS AND LICENSES

This project is licensed under the GNU GPLv2 license. fake6502 (the 6502 emulator core used in 6502golf) originally written by Mike Chambers, modified by omarandlorraine, is licensed under GNU GPLv2.

## RESOURCES

- fake6502 (the 6502 emulator core): [https://github.com/omarandlorraine/fake6502](https://github.com/omarandlorraine/fake6502)
- 64tass (6502 assembler): [https://github.com/irmen/64tass](https://github.com/irmen/64tass)
- Easy 6502 (for learning): [https://skilldrick.github.io/easy6502/](https://skilldrick.github.io/easy6502/)
