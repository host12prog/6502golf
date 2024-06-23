; echo.s: a simple example on
; how to use $FFE1 (charin)
* = $0200
main:
	lda #$0a ; send a newline
	sta $FFE0
	lda $FFE1 ; load charin
	sta $FFE0 ; store to charout
	lda #$0a ; for beauty purposes
	sta $FFE0 ; send a newline
	brk
empty_handler:
	rti
reset_handler:
	jmp main
* = $FFFA
	.word empty_handler
	.word reset_handler
	.word empty_handler
