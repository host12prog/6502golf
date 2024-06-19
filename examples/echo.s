* = $0200
main:
	lda $01 ; load charin
	sta $00 ; store to charout
	lda #$0a ; for beauty purposes
	sta $00 ; send a newline
	brk
