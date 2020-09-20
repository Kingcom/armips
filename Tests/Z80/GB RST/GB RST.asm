// Test Gameboy RST opcodes

.gb
.create "output.bin",0

	// RST 0-7 (Nintendo style)
	rst	0
	rst	1
	rst	2
	rst	3
	rst	4
	rst	5
	rst	6
	rst	7

	// RST 8, 16, 24, 32, 40, 48, 56 (common style)
	rst	0
	rst	8
	rst	16
	rst	24
	rst	32
	rst	40
	rst	48
	rst	56

.close
