.gba
.create "output.bin",0

.definelabel label,1234
.definelabel e,5678

// Should be parsed as exponentials
.notice 314e2
.notice 314e+2
.notice 314e-2
.notice 314E2
.notice 314E+2
.notice 314E-2

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
