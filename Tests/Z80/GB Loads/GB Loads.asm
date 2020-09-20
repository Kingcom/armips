// Test Gameboy load opcodes, aliases and alternatives

.gb
.create "output.bin",0

	// Load 8-bit immediate
	ld	a, 0x00
	ld	a, 0xFF
	ld	b, 0x00
	ld	b, 0xFF
	ld	c, 0x00
	ld	c, 0xFF
	ld	d, 0x00
	ld	d, 0xFF
	ld	e, 0x00
	ld	e, 0xFF
	ld	h, 0x00
	ld	h, 0xFF
	ld	l, 0x00
	ld	l, 0xFF

	// Load 16-bit immediate
	ld	bc, 0x0000
	ld	bc, 0xFFFF
	ld	de, 0x0000
	ld	de, 0xFFFF
	ld	hl, 0x0000
	ld	hl, 0xFFFF
	ld	sp, 0x0000
	ld	sp, 0xFFFF

	// Load and increment
	ld	(hli), a
	ld	(hl+), a
	ldi	(hl), a
	ld	a, (hli)
	ld	a, (hl+)
	ldi	a, (hl)

	// Load and decrement
	ld	(hld), a
	ld	(hl-), a
	ldd	(hl), a
	ld	a, (hld)
	ld	a, (hl-)
	ldd	a, (hl)

	// Load memory immediate
	ld	(0x0000), a
	ld	(0x1234), a
	ld	(0xFEFF), a
	ld	a, (0x0000)
	ld	a, (0x1234)
	ld	a, (0xFEFF)

	// Load high memory immediate
	ld	(0xFF00), a
	ld	(0xFF00+0x55), a
	ld	(0xFF55), a
	ldh	(0xFF00), a
	ldh	(0xFF00+0x55), a
	ldh	(0xFF55), a
	ldh	(0x55), a
	ld	a, (0xFF00)
	ld	a, (0xFF00+0x55)
	ld	a, (0xFF55)
	ldh	a, (0xFF00)
	ldh	a, (0xFF00+0x55)
	ldh	a, (0xFF55)
	ldh	a, (0x55)

	// Load high memory c
	ld	(0xFF00+c), a
	ld	(c), a
	ld	a, (0xFF00+c)
	ld	a, (c)

	// Load stack pointer
	ld	hl, sp+0x55
	ld	hl, sp-0x56
	ldhl	sp, 0x55
	ldhl	sp, -0x56

.close
