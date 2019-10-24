.macro test,label
	.notice °(label) + ": " + tohex(label,16)
.endmacro

.definelabel a1,0x00000000
.definelabel b1,0xFFFFFFFF
.definelabel c1,0x12345678
.definelabel d1,0xFFFFFFFFFFFFFFFF
.definelabel e1,0xFEDCBA9876543210
.definelabel f1,0x1234567890ABCDEF
.definelabel g1,0x1122334455667788

.gba
.create "output.bin",0

.org 0xDEADDEAD :: .org 0x00000000 :: a2:
.org 0xDEADDEAD :: .org 0xFFFFFFFF :: b2:
.org 0xDEADDEAD :: .org 0x12345678 :: c2:
.org 0xDEADDEAD :: .org 0xFFFFFFFFFFFFFFFF :: d2:
.org 0xDEADDEAD :: .org 0xFEDCBA9876543210 :: e2:
.org 0xDEADDEAD :: .org 0x1234567890ABCDEF :: f2:
.org 0xDEADDEAD :: .org 0x1122334455667788 :: g2:

test a0
test a1
test a2
test b0
test b1
test b2
test c0
test c1
test c2
test d0
test d1
test d2
test e0
test e1
test e2
test f0
test f1
test f2
test g0
test g1
test g2

.close
