.psx
.create "output.bin", 0

.region 20h
	.word 1,2,3,4
.endregion

.defineregion 80h,20h

.autoregion
.notice "Allocated A at " + tohex(.)
.word 5,6
.endautoregion

.autoregion 60h,100h
.notice "Allocated B at " + tohex(.)
.word 80h,81h
.endautoregion

.autoregion 0h,30h
.notice "Allocated C at " + tohex(.)
.word 9,10
.endautoregion

.autoregion 80h
.notice "Allocated D at " + tohex(.)
.word 82h,31h
.endautoregion

.close