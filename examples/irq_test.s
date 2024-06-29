; irq_test.s: prints out exactly 1 A
; once every 1/50th of a second

* = $0200
main:
  cli
  lda #$0d
  sta $FFE0
	lda #$0a ; newline
	sta $FFE0
	jmp * ; we better not BRK, we need IRQs
irq:
	pha
	lda $FFE2
	cmp #50 ; 50 ticks yet?
	bne noirq ; no then jump
	lda #0
	sta $FFE2 ; reset counter
	lda #$41 ; send character
	sta $FFE0
noirq:
	lda #0 ; acknowledge irq!!!
	sta $FFE3
	inc $40
	pla
	rti
empty_handler:
	rti
reset_handler:
	jmp main
* = $FFFA
	.word empty_handler
	.word reset_handler
	.word irq
