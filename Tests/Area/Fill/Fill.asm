.psx
.create "output.bin", 0

start:
.area 20h,1
	.word 0
.endarea

end:

.if (end-start) != 20h
	.error "Area content not filled"
.endif

// Perfectly filled area
.area 20h,2
	.fill 20h
.endarea

.close