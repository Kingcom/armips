.gba
.create "output.bin",0

; Align+fill
	.db		0x01
	.align		2, 0x22		; 1 byte
	.db		0x03
	.align		4, 0x44		; 1 byte
	.db		0x05
	.align		8, 0x66		; 3 bytes

; Align where it isn't needed
	.align		1, 0x77		; 0 bytes
	.align		2, 0x88		; 0 bytes
	.align		4, 0x99		; 0 bytes
	.align		8, 0xAA		; 0 bytes

; Default align
	.db		0xB
	.align				; 3 bytes

; Fill
	.fill		0x40, 0xCC	; 64 bytes
	.fill		0x40		; 64 bytes

; Align virtual address
	.headersize	2
	.org		0x80		; org=0x80, orga=0x7E
	.db		0xDD
	.align		4, 0xEE		; Produces 3 bytes
; Align physical address
	.aligna		4, 0xFF		; org=0x84, orga=0x82, so produces 2 bytes

.close
