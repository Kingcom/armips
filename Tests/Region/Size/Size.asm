.psx
.create "output.bin", 0

; Simple region, like area, with sufficient space.
.region 20h
	.word 1,2,3,4
.endregion

; A simple region with insufficient space.
.region 4h
	.word 5,6
.endregion

; Should fit.
.autoregion
.warning "Allocated at " + tohex(.)
.word 7,8,9
.endautoregion

; Won't fit.
.autoregion
.warning "Allocated at " + tohex(.)
.word 10,11
.endautoregion

.close