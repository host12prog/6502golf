; irq_test.s: prints out a block of As
; once every 1/50th of a second
; TO-DO: fix this so it only prints out 1 A
; at a time

* = $0200
	sei
	lda #<irq ; set IRQ vector
	sta $fffe ; to our IRQ routine
	lda #>irq
	sta $ffff
	lda #$0a ; newline
	sta $00
	lda #0
	sta $40 ; $40 is our timer
	cli
	jmp * ; we better not BRK, we need IRQs
irq:
	pha
	lda $40
	cmp #50 ; 50 ticks yet?
	bne noirq ; no then jump
	lda #$41 ; send character
	sta $00
noirq:
	inc $40
	pla
	rti
