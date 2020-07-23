.psx
.create "output.bin", 0

start equ 0
.defineregion start,20h,1
end:

.if (end-start) != 20h
	;;.error "Area content not filled"
.endif

.close