.gba
.create "output.bin",0

; Invalid align
	.align		3
	.aligna		3
; But fill works fine
	.fill		3, 0x11
	.fill		3

.close
