.psx
.create "output.bin",0

.definelabel memory1,0x123456
.definelabel memory2,0x128456

; li

li	a0,123456h			; standard li
li	a0,0FFFFF123h			; li that turns into addiu from r0
li	a0,12000000h			; only lui
li	a0,7FF0h			; li that turns into ori from r0

; load/store
lb	a0,123456h
lbu	a0,0FFFFF123h
lh	a0,12000000h
lhu	a0,7FF0h
lw	a0,7FF0h

.resetdelay

sb	a0,123456h
sh	a0,0FFFFF123h
sw	a0,7FF0h

lb.u	a0,1234h			; should turn into nop
lb.l	a0,123456h			; should just be the second instruction

; load/store unaligned
ulh		a0,1234h(a1)
ulhu	a0,1234h(a0)
ulw		a0,1234h(a1)
ush		a0,1234h(a1)
usw		a0,1234h(a1)


.close
