; echo.s: a simple example on
; how to use $01 (charin)
* = $0200
	sei
main:
	lda #$0a ; send a newline
	sta $00
	lda $01 ; load charin
	sta $00 ; store to charout
	lda #$0a ; for beauty purposes
	sta $00 ; send a newline
	brk
