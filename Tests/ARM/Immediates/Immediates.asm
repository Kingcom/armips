.nds
.create "output.bin",0


.thumb

	mov		r1,0x00			; min
	mov		r2,0xFF			; max
;	mov		r3,-0x1			; error: below min
;	mov		r4,0x100		; error: above max

	cmp		r1,0x00			; min
	cmp		r2,0xFF			; max
;	cmp		r3,-0x1			; error: below min
;	cmp		r4,0x100		; error: above max

	add		r1,0x00			; min
	add		r2,0xFF			; max
	add		r3,-0x1			; min converted to sub
	add		r4,-0xFF		; max converted to sub
;	add		r5,0x100		; error: above max
;	add		r6,-0x100		; error: above max after conversion

	sub		r1,0x00			; min
	sub		r2,0xFF			; max
	sub		r3,-0x1			; min converted to add
	sub		r4,-0xFF		; max converted to add
;	sub		r5,0x100		; error: above max
;	sub		r6,-0x100		; error: above max after conversion

	add		r1,r2,0x0		; min
	add		r2,r3,0x7		; max
	add		r3,r4,-0x1		; min converted to sub
	add		r4,r5,-0x7		; max converted to sub
;	add		r5,r6,0x8		; error: above max
;	add		r6,r7,-0x8		; error: above max after conversion

	sub		r1,r2,0x0		; min
	sub		r2,r3,0x7		; max
	sub		r3,r4,-0x1		; min converted to add
	sub		r4,r5,-0x7		; max converted to add
;	sub		r5,r6,0x8		; error: above max
;	sub		r6,r7,-0x8		; error: above max after conversion

	add		sp,0x0			; min
	add		sp,0x1FC		; max
	add		sp,-0x4			; min converted to sub
	add		sp,-0x1FC		; max converted to sub
;	add		sp,0x200		; error: above max
;	add		sp,-0x200		; error: above max after conversion

	sub		sp,0x0			; min
	sub		sp,0x1FC		; max
	sub		sp,-0x4			; min converted to add
	sub		sp,-0x1FC		; max converted to add
;	sub		sp,0x200		; error: above max
;	sub		sp,-0x200		; error: above max after conversion


.align 4
.arm

	and		r1,r2,0x00		; min bits
	and		r2,r3,0xFF		; max bits
	and		r3,r4,~0x0		; min bits converted to bic
	and		r4,r5,~0xFF		; max bits converted to bic
;	and		r5,r6,0x1FF		; error: above max bits
;	and		r6,r7,~0x1FF	; error: above max bits after conversion

	eor		r1,r2,0x00		; min bits
	eor		r2,r3,0xFF		; max bits
;	eor		r3,r4,~0x0		; error: not converted
;	eor		r4,r5,0x1FF		; error: above max bits

	sub		r1,r2,0x00		; min bits
	sub		r2,r3,0xFF		; max bits
	sub		r3,r4,-0x1		; min bits converted to add
	sub		r4,r5,-0xFF		; max bits converted to add
;	sub		r5,r6,0x1FF		; error: above max bits
;	sub		r6,r7,-0x1FF	; error: above max bits after conversion

	rsb		r1,r2,0x00		; min bits
	rsb		r2,r3,0xFF		; max bits
;	rsb		r3,r4,-0x1		; error: not converted
;	rsb		r4,r5,0x1FF		; error: above max bits

	add		r1,r2,0x00		; min bits
	add		r2,r3,0xFF		; max bits
	add		r3,r4,-0x1		; min bits converted to sub
	add		r4,r5,-0xFF		; max bits converted to sub
;	add		r5,r6,0x1FF		; error: above max bits
;	add		r6,r7,-0x1FF	; error: above max bits after conversion

	adc		r1,r2,0x00		; min bits
	adc		r2,r3,0xFF		; max bits
;	adc		r3,r4,-0x1		; error: not converted
;	adc		r4,r5,0x1FF		; error: above max bits

	sbc		r1,r2,0x00		; min bits
	sbc		r2,r3,0xFF		; max bits
;	sbc		r3,r4,-0x1		; error: not converted
;	sbc		r4,r5,0x1FF		; error: above max bits

	rsc		r1,r2,0x00		; min bits
	rsc		r2,r3,0xFF		; max bits
;	rsc		r3,r4,-0x1		; error: not converted
;	rsc		r4,r5,0x1FF		; error: above max bits

	tst		r1,0x00			; min bits
	tst		r2,0xFF			; max bits
;	tst		r3,~0x0			; error: not converted
;	tst		r4,0x1FF		; error: above max bits

	teq		r1,0x00			; min bits
	teq		r2,0xFF			; max bits
;	teq		r3,~0x0			; error: not converted
;	teq		r4,0x1FF		; error: above max bits

	cmp		r1,0x00			; min bits
	cmp		r2,0xFF			; max bits
	cmp		r3,-0x1			; min bits converted to cmn
	cmp		r4,-0xFF		; max bits converted to cmn
;	cmp		r5,0x1FF		; error: above max bits
;	cmp		r6,-0x1FF		; error: above max bits after conversion

	cmn		r1,0x00			; min bits
	cmn		r2,0xFF			; max bits
	cmn		r3,-0x1			; min bits converted to cmp
	cmn		r4,-0xFF		; max bits converted to cmp
;	cmn		r5,0x1FF		; error: above max bits
;	cmn		r6,-0x1FF		; error: above max bits after conversion

	orr		r1,r2,0x00		; min bits
	orr		r2,r3,0xFF		; max bits
;	orr		r3,r4,~0x0		; error: not converted
;	orr		r3,r5,0x1FF		; error: above max bits

	mov		r1,0x00			; min bits
	mov		r2,0xFF			; max bits
	mov		r3,~0x0			; min bits converted to mvn
	mov		r4,~0xFF		; max bits converted to mvn
;	mov		r5,0x1FF		; error: above max bits
;	mov		r6,-0x1FF		; error: above max bits after conversion

	bic		r1,r2,0x00		; min bits
	bic		r2,r3,0xFF		; max bits
	bic		r3,r4,~0x0		; min bits converted to and
	bic		r4,r5,~0xFF		; max bits converted to and
;	bic		r5,r6,0x1FF		; error: above max bits
;	bic		r6,r7,~0x1FF	; error: above max bits after conversion

	mvn		r1,0x00			; min bits
	mvn		r2,0xFF			; max bits
	mvn		r3,~0x0			; min bits converted to mov
	mvn		r4,~0xFF		; max bits converted to mov
;	mvn		r5,0x1FF		; error: above max bits
;	mvn		r6,-0x1FF		; error: above max bits after conversion

	ldr		r1,=12345678h	; test pool loads
	ldr		r2,=12345678h	; test repeated load of the same immediate
	.pool

	ldr		r3,=8800h		; test ldr conversion to mov
	ldr		r4,=0FFFFFFFFh	; test ldr conversion to mvn
	ldr		r5,=1FF00h		; test not converted to mov
	ldr		r6,=0FFFFFE00h	; test not converted to mvn
	.pool

	; test shifted immediates
.macro simm,reg,imm,rot
	mov	reg,(imm >> rot) | (imm <<  (32-rot))
.endmacro

	simm	r7,0FFh,0
	simm	r7,0FFh,2
	simm	r7,0FFh,4
	simm	r7,0FFh,6
	simm	r7,0FFh,8
	simm	r7,0FFh,10
	simm	r7,0FFh,12
	simm	r7,0FFh,14
	simm	r7,0FFh,16
	simm	r7,0FFh,18
	simm	r7,0FFh,20
	simm	r7,0FFh,22
	simm	r7,0FFh,24
	simm	r7,0FFh,26
	simm	r7,0FFh,28
	simm	r7,0FFh,30


.close