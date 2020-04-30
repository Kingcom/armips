.gba
.create "output.bin",0

.definelabel label,1234
.definelabel e,5678

// Should be parsed as exponentials
.notice 628e2
.notice 628e+2
.notice 628e-2
.notice 628E2
.notice 628E+2
.notice 628E-2
.notice 6.28e2
.notice 6.28e+2
.notice 6.28e-2
.notice 6.28E2
.notice 6.28E+2
.notice 6.28E-2

// Should be parsed as floats, but NOT as exponentials
.notice 0x0.e
.notice 0x0.E
.notice 0x0.E8
.notice 0xe.4
.notice 0xE.4
.notice 0xe.e
.notice 0xE.E
.notice 0xE.E8

// Should NOT be parsed as exponentials
.notice 0xE2
.notice 0xE+2
.notice 0xE-2
.notice 0xE+label
.notice 0xE-label
.notice 0xE
.notice 0Eh
.notice 0Eh+label
.notice 0Eh-label
.notice e
.notice e+2
.notice e-2

.close
