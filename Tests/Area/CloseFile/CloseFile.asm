.gba
.create "out.bin",0

.area 0x8
.fill 0x8
.close
.endarea

.macro myopen,offset
	.open "out2.bin",offset
.endmacro

.area 0x8
myopen 4
.endarea

.close
