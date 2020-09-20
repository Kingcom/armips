.ereader
.create "output.bin",0

	// 00 - FF
	nop
	ld	bc, 0x1234
	ld	(bc), a
	inc	bc
	inc	b
	dec	b
	ld	b, 0x55
	rlca
	add	hl, bc
	ld	a, (bc)
	dec	bc
	inc	c
	dec	c
	ld	c, 0x55
	rrca
	djnz	.
	ld	de, 0x1234
	ld	(de), a
	inc	de
	inc	d
	dec	d
	ld	d, 0x55
	rla
	jr	.
	add	hl, de
	ld	a, (de)
	dec	de
	inc	e
	dec	e
	ld	e, 0x55
	rra
	jr	nz, .
	ld	hl, 0x1234
	ld	(0x1234), hl
	inc	hl
	inc	h
	dec	h
	ld	h, 0x55
	jr	z, .
	add	hl, hl
	ld	hl, (0x1234)
	dec	hl
	inc	l
	dec	l
	ld	l, 0x55
	cpl
	jr	nc, .
	ld	sp, 0x1234
	ld	(0x1234), a
	inc	sp
	inc	(hl)
	dec	(hl)
	ld	(hl), 0x55
	scf
	jr	c, .
	add	hl, sp
	ld	a, (0x1234)
	dec	sp
	inc	a
	dec	a
	ld	a, 0x55
	ccf
	ld	b, b
	ld	b, c
	ld	b, d
	ld	b, e
	ld	b, h
	ld	b, l
	ld	b, (hl)
	ld	b, a
	ld	c, b
	ld	c, c
	ld	c, d
	ld	c, e
	ld	c, h
	ld	c, l
	ld	c, (hl)
	ld	c, a
	ld	d, b
	ld	d, c
	ld	d, d
	ld	d, e
	ld	d, h
	ld	d, l
	ld	d, (hl)
	ld	d, a
	ld	e, b
	ld	e, c
	ld	e, d
	ld	e, e
	ld	e, h
	ld	e, l
	ld	e, (hl)
	ld	e, a
	ld	h, b
	ld	h, c
	ld	h, d
	ld	h, e
	ld	h, h
	ld	h, l
	ld	h, (hl)
	ld	h, a
	ld	l, b
	ld	l, c
	ld	l, d
	ld	l, e
	ld	l, h
	ld	l, l
	ld	l, (hl)
	ld	l, a
	ld	(hl), b
	ld	(hl), c
	ld	(hl), d
	ld	(hl), e
	ld	(hl), h
	ld	(hl), l
	wait	a
	ld	(hl), a
	ld	a, b
	ld	a, c
	ld	a, d
	ld	a, e
	ld	a, h
	ld	a, l
	ld	a, (hl)
	ld	a, a
	add	a, b
	add	a, c
	add	a, d
	add	a, e
	add	a, h
	add	a, l
	add	a, (hl)
	add	a, a
	adc	a, b
	adc	a, c
	adc	a, d
	adc	a, e
	adc	a, h
	adc	a, l
	adc	a, (hl)
	adc	a, a
	sub	b
	sub	c
	sub	d
	sub	e
	sub	h
	sub	l
	sub	(hl)
	sub	a
	sbc	a, b
	sbc	a, c
	sbc	a, d
	sbc	a, e
	sbc	a, h
	sbc	a, l
	sbc	a, (hl)
	sbc	a, a
	and	b
	and	c
	and	d
	and	e
	and	h
	and	l
	and	(hl)
	and	a
	xor	b
	xor	c
	xor	d
	xor	e
	xor	h
	xor	l
	xor	(hl)
	xor	a
	or	b
	or	c
	or	d
	or	e
	or	h
	or	l
	or	(hl)
	or	a
	cp	b
	cp	c
	cp	d
	cp	e
	cp	h
	cp	l
	cp	(hl)
	cp	a
	ret	nz
	pop	bc
	jp	nz, 0x1234
	jp	0x1234
	call	nz, 0x1234
	push	bc
	add	a, 0x55
	rst	0x0
	ret	z
	ret
	jp	z, 0x1234
	call	z, 0x1234
	call	0x1234
	adc	a, 0x55
	rst	0x8
	ret	nc
	pop	de
	jp	nc, 0x1234
	wait	0x55
	call	nc, 0x1234
	push	de
	sub	0x55
	ret	c
	jp	c, 0x1234
	call	c, 0x1234
	sbc	a, 0x55
	pop	hl
	ex	(sp), hl
	push	hl
	and	0x55
	jp	(hl)
	ex	de, hl
	xor	0x55
	ret	p
	pop	af
	jp	p, 0x1234
	push	af
	or	0x55
	ret	m
	ld	sp, hl
	jp	m, 0x1234
	cp	0x55

.close
