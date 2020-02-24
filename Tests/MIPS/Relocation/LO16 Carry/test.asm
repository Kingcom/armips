.text
.globl test

test:
	lbu	$v0,value+4
	jr	$ra
	nop
