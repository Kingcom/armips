.text
.globl test

.set noreorder

// This file will generate the following relocations:
// - two consecutive R_MIPS_HI16 for the symbol "value", with an offset of 1
// - one R_MIPS_LO16 for the symbol "value", with an offset of 4

test:
	move	$v1,$0
	beqzl	$a0,end
	lui	$v0,%hi(value+0x10004)
	li	$v1,1
	lui	$v0,%hi(value+0x10004)
end:
	jr	$ra
	addiu	$v0,%lo(value+0x10004)
