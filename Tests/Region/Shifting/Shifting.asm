.psx
.create "output.bin", 0x80010000

.org 0x8001FFF8
.region 32
.endregion

.autoregion
	li	v0,data
.endautoregion

.autoregion
data: .word 1234
.endautoregion

.org 0x8002FFF8
.region 32
	li	v0,data2
.endregion

.autoregion
data2: .word 1234
.endautoregion

.close
