.saturn
.create "output.bin",0
	mov #0xCC, r0
	mov #0xCC, r1
	mov #0xCC, r2
	mov #0xCC, r3
	mov #0xCC, r4
	mov #0xCC, r5
	mov #0xCC, r6
	mov #0xCC, r7
	mov #0xCC, r8
	mov #0xCC, r9
	mov #0xCC, r10
	mov #0xCC, r11
	mov #0xCC, r12
	mov #0xCC, r13
	mov #0xCC, r14
	mov #0xCC, r15
	
	mov.w Immediate16BitValue, r0
	mov.w Immediate16BitValue, r1
	mov.w Immediate16BitValue, r2
	mov.w Immediate16BitValue, r3
	mov.w Immediate16BitValue, r4
	mov.w Immediate16BitValue, r5
	mov.w Immediate16BitValue, r6
	mov.w Immediate16BitValue, r7
	mov.w Immediate16BitValue, r8
	mov.w Immediate16BitValue, r9
	mov.w Immediate16BitValue, r10
	mov.w Immediate16BitValue, r11
	mov.w Immediate16BitValue, r12
	mov.w Immediate16BitValue, r13
	mov.w Immediate16BitValue, r14
	mov.w Immediate16BitValue, r15
	nop
	
	.align 2
	Immediate16BitValue: .dw 0xCCCC

	mov.l Immediate32BitValue, r0
	mov.l Immediate32BitValue, r1
	mov.l Immediate32BitValue, r2
	mov.l Immediate32BitValue, r3
	mov.l Immediate32BitValue, r4
	mov.l Immediate32BitValue, r5
	mov.l Immediate32BitValue, r6
	mov.l Immediate32BitValue, r7
	mov.l Immediate32BitValue, r8
	mov.l Immediate32BitValue, r9
	mov.l Immediate32BitValue, r10
	mov.l Immediate32BitValue, r11
	mov.l Immediate32BitValue, r12
	mov.l Immediate32BitValue, r13
	mov.l Immediate32BitValue, r14
	mov.l Immediate32BitValue, r15

	.align 4
	Immediate32BitValue: .dd 0xCCCCCCCC

	mov.w @(4,pc), r0
	mov.w @(4,pc), r1
	mov.w @(4,pc), r2
	mov.w @(4,pc), r3
	mov.w @(4,pc), r4
	mov.w @(4,pc), r5
	mov.w @(4,pc), r6
	mov.w @(4,pc), r7
	mov.w @(4,pc), r8
	mov.w @(4,pc), r9
	mov.w @(4,pc), r10
	mov.w @(4,pc), r11
	mov.w @(4,pc), r12
	mov.w @(4,pc), r13
	mov.w @(4,pc), r14
	mov.w @(4,pc), r15
	
	mov.l @(4,pc), r0
	mov.l @(2,pc), r1
	mov.l @(4,pc), r2
	mov.l @(2,pc), r3
	mov.l @(4,pc), r4
	mov.l @(2,pc), r5
	mov.l @(4,pc), r6
	mov.l @(2,pc), r7
	mov.l @(4,pc), r8
	mov.l @(2,pc), r9
	mov.l @(4,pc), r10
	mov.l @(2,pc), r11
	mov.l @(4,pc), r12
	mov.l @(2,pc), r13
	mov.l @(4,pc), r14
	mov.l @(2,pc), r15
	
	mov r0, r1
	mov r0, r2
	mov r0, r3
	mov r0, r4
	mov r0, r5
	mov r0, r6
	mov r0, r7
	mov r0, r8
	mov r0, r9
	mov r0, r10
	mov r0, r11
	mov r0, r12
	mov r0, r13
	mov r0, r14
	mov r0, r15
	mov r1, r0
	mov r1, r2
	mov r1, r3
	mov r1, r4
	mov r1, r5
	mov r1, r6
	mov r1, r7
	mov r1, r8
	mov r1, r9
	mov r1, r10
	mov r1, r11
	mov r1, r12
	mov r1, r13
	mov r1, r14
	mov r1, r15
	mov r2, r0
	mov r2, r1
	mov r2, r3
	mov r2, r4
	mov r2, r5
	mov r2, r6
	mov r2, r7
	mov r2, r8
	mov r2, r9
	mov r2, r10
	mov r2, r11
	mov r2, r12
	mov r2, r13
	mov r2, r14
	mov r2, r15
	mov r3, r0
	mov r3, r1
	mov r3, r2
	mov r3, r4
	mov r3, r5
	mov r3, r6
	mov r3, r7
	mov r3, r8
	mov r3, r9
	mov r3, r10
	mov r3, r11
	mov r3, r12
	mov r3, r13
	mov r3, r14
	mov r3, r15
	mov r4, r0
	mov r4, r1
	mov r4, r2
	mov r4, r3
	mov r4, r5
	mov r4, r6
	mov r4, r7
	mov r4, r8
	mov r4, r9
	mov r4, r10
	mov r4, r11
	mov r4, r12
	mov r4, r13
	mov r4, r14
	mov r4, r15
	mov r5, r0
	mov r5, r1
	mov r5, r2
	mov r5, r3
	mov r5, r4
	mov r5, r6
	mov r5, r7
	mov r5, r8
	mov r5, r9
	mov r5, r10
	mov r5, r11
	mov r5, r12
	mov r5, r13
	mov r5, r14
	mov r5, r15
	mov r6, r0
	mov r6, r1
	mov r6, r2
	mov r6, r3
	mov r6, r4
	mov r6, r5
	mov r6, r7
	mov r6, r8
	mov r6, r9
	mov r6, r10
	mov r6, r11
	mov r6, r12
	mov r6, r13
	mov r6, r14
	mov r6, r15
	mov r7, r0
	mov r7, r1
	mov r7, r2
	mov r7, r3
	mov r7, r4
	mov r7, r5
	mov r7, r6
	mov r7, r8
	mov r7, r9
	mov r7, r10
	mov r7, r11
	mov r7, r12
	mov r7, r13
	mov r7, r14
	mov r7, r15
	mov r8, r0
	mov r8, r1
	mov r8, r2
	mov r8, r3
	mov r8, r4
	mov r8, r5
	mov r8, r6
	mov r8, r7
	mov r8, r9
	mov r8, r10
	mov r8, r11
	mov r8, r12
	mov r8, r13
	mov r8, r14
	mov r8, r15
	mov r9, r0
	mov r9, r1
	mov r9, r2
	mov r9, r3
	mov r9, r4
	mov r9, r5
	mov r9, r6
	mov r9, r7
	mov r9, r8
	mov r9, r10
	mov r9, r11
	mov r9, r12
	mov r9, r13
	mov r9, r14
	mov r9, r15
	mov r10, r0
	mov r10, r1
	mov r10, r2
	mov r10, r3
	mov r10, r4
	mov r10, r5
	mov r10, r6
	mov r10, r7
	mov r10, r8
	mov r10, r9
	mov r10, r11
	mov r10, r12
	mov r10, r13
	mov r10, r14
	mov r10, r15
	mov r11, r0
	mov r11, r1
	mov r11, r2
	mov r11, r3
	mov r11, r4
	mov r11, r5
	mov r11, r6
	mov r11, r7
	mov r11, r8
	mov r11, r9
	mov r11, r10
	mov r11, r12
	mov r11, r13
	mov r11, r14
	mov r11, r15
	mov r12, r0
	mov r12, r1
	mov r12, r2
	mov r12, r3
	mov r12, r4
	mov r12, r5
	mov r12, r6
	mov r12, r7
	mov r12, r8
	mov r12, r9
	mov r12, r10
	mov r12, r11
	mov r12, r13
	mov r12, r14
	mov r12, r15
	mov r13, r0
	mov r13, r1
	mov r13, r2
	mov r13, r3
	mov r13, r4
	mov r13, r5
	mov r13, r6
	mov r13, r7
	mov r13, r8
	mov r13, r9
	mov r13, r10
	mov r13, r11
	mov r13, r12
	mov r13, r14
	mov r13, r15
	mov r14, r0
	mov r14, r1
	mov r14, r2
	mov r14, r3
	mov r14, r4
	mov r14, r5
	mov r14, r6
	mov r14, r7
	mov r14, r8
	mov r14, r9
	mov r14, r10
	mov r14, r11
	mov r14, r12
	mov r14, r13
	mov r14, r15
	mov r15, r0
	mov r15, r1
	mov r15, r2
	mov r15, r3
	mov r15, r4
	mov r15, r5
	mov r15, r6
	mov r15, r7
	mov r15, r8
	mov r15, r9
	mov r15, r10
	mov r15, r11
	mov r15, r12
	mov r15, r13
	mov r15, r14
	
	mov.b r5, @r8
	mov.w r5, @r8
	mov.l r5, @r8
	
	mov.b @r5, r8
	mov.w @r5, r8
	mov.l @r5, r8
	
	mov.b r5, @-r8
	mov.w r5, @-r8
	mov.l r5, @-r8
	
	mov.b @r5+, r8
	mov.w @r5+, r8
	mov.l @r5+, r8
	
	mov.b r0, @(4,r8)
	mov.w r0, @(4,r8)
	mov.l r5, @(4,r8)
	
	mov.b @(4,r8), r0
	mov.w @(4,r8), r0
	mov.l @(4,r8), r5
	
	mov.b r5,@(r0,r5)
	mov.w r5,@(r0,r5)
	mov.l r5,@(r0,r5)
	
	mov.b @(r0,r5), r5
	mov.w @(r0,r5), r5
	mov.l @(r0,r5), r5
	
	mov.b r0, @(0x50,gbr)
	mov.w r0, @(0x50,gbr)
	mov.l r0, @(0x50,gbr)
	
	mov.b @(0x50,gbr), r0
	mov.w @(0x50,gbr), r0
	mov.l @(0x50,gbr), r0
	
	nop
	mova Somewhere, r0
	mova @(4,pc),r0
	
	movt r3
	swap.b r5, r8
	swap.w r5, r8
	xtrct r5, r8
	add r5, r8
	add #0x55, r8
	addc r5, r8
	addv r5, r8
	
	cmp/eq #0x05,r0
	cmp/eq r5,r8
	cmp/hs r5,r8
	cmp/ge r5,r8
	cmp/hi r5,r8
	cmp/gt r5,r8
	cmp/pl r5
	cmp/pz r8
	cmp/str r5,r8
	div1 r5,r8
	div0s r5,r8
	div0u

	dmuls.l r5,r8
	dmulu.l r5,r8
	dt r5
	exts.b r5,r8
	exts.w r5,r8
	extu.b r5,r8
	extu.w r5,r8
	mac.l @r5+,@r8+
	mac.w @r5+,@r8+
	mul.l r5,r8
	muls.w r5,r8
	mulu.w r5,r8
	neg r5,r8
	negc r5,r8
	sub r5,r8
	subc r5,r8
	subv r5,r8

Somewhere:
	and r5,r8
	and #0x7F,r0
	and.b #0x7F,@(r0,gbr)
	not r5,r8
	or r5,r8
	or #0x50,r0
	or.b #0x50,@(r0,gbr)
	tas.b @r5
	tst r5,r8
	tst #0x50,r0
	tst.b #0x50,@(r0,gbr)
	xor r5,r8
	xor #0x50,r0
	xor.b #0x50,@(r0,gbr)
	
	rotl r5
	rotr r5
	rotcl r5
	rotcr r5
	shal r5
	shar r5
	shll r5
	shlr r5
	shll2 r5
	shlr2 r5
	shll8 r5
	shlr8 r5
	shll16 r5
	shlr16 r5
	
	bf Somewhere
	bf/s Somewhere
	bt Somewhere
	bt/s Somewhere
	bra Somewhere
	braf r5
	bsr Somewhere
	bsrf r5
	jmp r5
	jsr r5
	rts
	
	clrt
	clrmac
	
	ldc r5,sr
	ldc r5,gbr
	ldc r5,vbr
	ldc.l @r5+,sr
	ldc.l @r5+,gbr
	ldc.l @r5+,vbr
	lds r5,mach
	lds r5,macl
	lds r5,pr
	lds.l @r5+,mach
	lds.l @r5+,macl
	lds.l @r5+,pr
	
	nop
	rte
	sett
	sleep
	
	stc sr,r5
	stc gbr,r5
	stc vbr,r5
	stc.l sr,@-r5
	stc.l gbr,@-r5
	stc.l vbr,@-r5
	sts mach,r5
	sts macl,r5
	sts pr,r5
	sts.l mach,@-r5
	sts.l macl,@-r5
	sts.l pr,@-r5
	
	trapa #5


.close
