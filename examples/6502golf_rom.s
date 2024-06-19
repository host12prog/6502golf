* = $0200
main:
	lda text,y
	cmp #0
	beq done
	sta $00
	iny
	jmp main
done:
	brk
text:
	.text "6502GOLF TEST ROM",$0A,$00
