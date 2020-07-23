.psx
.create "output.bin", 0

.region 20h
	.word 1,2,3,4
.endregion

.autoregion
.notice "Allocated at " + tohex(.)
.word 5
.endautoregion

.autoregion
.notice "Allocated at " + tohex(.)
.word 6
.endautoregion

.autoregion
.notice "Allocated at " + tohex(.)
.word 7
.endautoregion

.autoregion
.notice "Allocated at " + tohex(.)
.word 8
.endautoregion

.close