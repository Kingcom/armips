.psp
.create "output.bin", 0

				; PSP opcodes

ll	a1,4(a2)
lwc1	f1,(a2)

lv.s	S123,0x20(s0)
lv.s	S321,(s0)

ulv.q	C220,0x40(s1)
ulv.q	C222,0x40(s1)

lvl.q	C220,0x40(s1)
lvr.q	C220,0x40(s1)

lv.q	C530,0x40(s1)
lv.q	C530,(s1)

sc	a1,4(a2)
swc1	f1,(a2)

sv.s	S123,0x20(s0)
sv.s	S321,(s0)

usv.q	C220,0x40(s1)
usv.q	C222,0x40(s1)

svl.q	C220,0x40(s1)
svr.q	C220,0x40(s1)

sv.q	C530,0x40(s1)
sv.q	C530,(s1)
sv.q	C530,0x40(s1), wb
sv.q	C530,(s1), wb

				; Special

rotr	a1,a2,3h
rotr	a1,3h
rotrv	a1,a2,a3
rotrv	a1,a2
clo	a1,a2
clz	a1,a2
madd	a1,a2
maddu	a1,a2
max	a1,a2,a3
min	a1,a2,a3
msub	a1,a2
msubu	a1,a2


				; VFPU0
vadd.s	S100,S220,S333
vsub.p	R122,C430,C010
vcrsp.t	c121,C430,C010
vdiv.q	C120,C430,C010

				; VFPU1
vscl.t	C100,C220,S333

				; VFPU3
vcmp.s	GT,S100,S101

				; VFPU5
vpfxs	x,-X,-|y|,w
vpfxt	-3,1/4,1/6,0
vpfxt	1/2,2,1,1/3
vpfxd	[0:1],m,[-1:1],
vpfxd	,[0:1],[-1:1],M

				; typical representable half floats
vfim.s	S123,0.0
vfim.s	S123,20.0
				; 2^-14 is least representable half float
vfim.s	S123,0.00006103515625
				; 2^-15 would've been the least subnormal, but flushes to 0
vfim.s	S123,0.000030517578125
				; smaller numbers all round to 0
vfim.s	S123,0.00001
				; 2^15 - 2^5 is greatest representable half float
vfim.s	S123,65504.0
				; any number greater than that is nearer to that than infinity, so round down
vfim.s	S123,1000000.0

				; VFPU4-1.2
vf2h.p	S102,C002
vf2h.q	C102,C200
vh2f.s	C102,S203
vh2f.p	C100,C202
vc2i.s	C100,S203
vuc2ifs.s	C100,S203
vus2i.s	C102,S203
vus2i.p	C100,C202
vs2i.s	C102,S203
vs2i.p	C100,C202
vi2uc.q	S103,C200
vi2c.q	S103,C200
vi2us.p	S103,C202
vi2us.q	C102,C200
vi2s.p	S103,C202
vi2s.q	C102,C200

				; VFPU4-1.3
vt4444.q	C102,C200
vt5551.q	C102,C200
vt5650.q	C102,C200

				; VF6-1.1
vrot.p	C102,S200,[C,-S]
vrot.t	C101,S200,[C,0,-S]
vrot.q	C100,S200,[0,C,-S,0]

.close
