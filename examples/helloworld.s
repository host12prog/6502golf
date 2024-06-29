* = $0200
main:
	lda #$0D
	sta $FFE0
	lda #$0A
	sta $FFE0
loop:
	lda text,y
	cmp #0
	beq done
	sta $FFE0
	iny
	jmp loop
done:
	jmp done ; get stuck here
text:
	.text "Hello world!",$0A,$00
empty_handler:
	rti
reset_handler:
	jmp main
* = $FFFA
	.word empty_handler
	.word reset_handler
	.word empty_handler
