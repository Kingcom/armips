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
vdot.p	S113,C202,C202
vdot.t	S301,C110,C110
vdot.t	S223,C201,C301
vdot.q	S333,C200,C200
vhdp.p	S113,C202,C202
vhdp.t	S223,C201,C201
vhdp.q	S333,C200,C200
vdet.p	S113,C202,C212
vcrs.t	C101,C111,C121

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
vavg.p	S103,C202
vavg.t	S103,C201
vavg.q	S103,C200
vfad.p	S103,C202
vfad.t	S103,C201
vfad.q	S103,C200
vt4444.q	C102,C200
vt5551.q	C102,C200
vt5650.q	C102,C200

				; VF6-1.1
vrot.p	C102,S200,[C,-S]
vrot.t	C101,S200,[C,0,-S]
vrot.q	C100,S200,[0,C,-S,0]

				; pspautotests VFPU instructions
mfvc	v0,CC
mtvc	v0,CC
vabs.p	C000,C100
vabs.q	C000,C100
vabs.s	S000,S100
vabs.t	C000,C100
vadd.p	C000,C100,C200
vadd.q	C000,C100,C200
vadd.q	R000,R000,R100
vadd.s	S000,S100,S200
vadd.t	C000,C100,C200
vadd.t	R600,R600,R500
vadd.t	R601,R601,R501
vadd.t	R602,R602,R502
vasin.p	C000,C100
vasin.q	C000,C100
vasin.s	S000,S100
vasin.t	C000,C100
vavg.p	S000,C100
vavg.q	S000,C100
vavg.t	S000,C100
vbfy1.p	C000,C100
vbfy1.q	C000,C100
vbfy2.q	C000,C100
vcmovt.s	S100,S000,0
vcmovt.s	S110,S000,1
vcmovt.s	S120,S000,2
vcmovt.s	S130,S000,3
vcmp.q	EI,R500,R600
vcmp.q	EN,R500,R600
vcmp.q	EQ,R500,R600
vcmp.q	ES,R500,R600
vcmp.q	EZ,R500,R600
vcmp.q	FL,R500,R600
vcmp.q	GE,R500,R600
vcmp.q	GT,R500,R600
vcmp.q	LE,R500,R600
vcmp.q	LT,R500,R600
vcmp.q	NE,R500,R600
vcmp.q	NI,R500,R600
vcmp.q	NN,R500,R600
vcmp.q	NS,R500,R600
vcmp.q	NZ,R500,R600
vcmp.q	TR,R500,R600
vcmp.t	EQ,R500,R600
vcos.p	C000,C100
vcos.q	C000,C100
vcos.s	S000,S100
vcos.t	C000,C100
vcrs.t	C000,C100,C200
vcrsp.t	C000,C100,C200
vcst.s	S000,maxfloat		; VFPU_HUGE
vcst.s	S001,2/pi			; VFPU_2_PI
vcst.s	S002,pi				; VFPU_PI
vcst.s	S003,ln(2)			; VFPU_LN2
vcst.s	S010,sqrt(2)		; VFPU_SQRT2
vcst.s	S011,1/pi			; VFPU_1_PI
vcst.s	S012,e				; VFPU_E
vcst.s	S013,ln(10)			; VFPU_LN10
vcst.s	S020,sqrt(1/2)		; VFPU_SQRT1_2
vcst.s	S021,pi/4			; VFPU_PI_4
vcst.s	S022,log2(e)		; VFPU_LOG2E
vcst.s	S023,2*pi			; VFPU_2PI
vcst.s	S030,2/sqrt(pi)		; VFPU_2_SQRTPI
vcst.s	S031,pi/2			; VFPU_PI_2
vcst.s	S032,log10(e)		; VFPU_LOG10E
vcst.s	S033,pi/6			; VFPU_PI_6
vcst.s	S100,log10(2)		; VFPU_LOG10TWO
vcst.s	S110,log2(10)		; VFPU_LOG2TEN
vcst.s	S120,sqrt(3)/2		; VFPU_SQRT3_2
vdet.p	S000,C100,C200
vdiv.p	C000,C100,C200
vdiv.q	C000,C100,C200
vdiv.s	S000,S100,S200
vdiv.t	C000,C100,C200
vdot.p	S000,C100,C200
vdot.q	S000,C100,C200
vdot.t	S000,C100,C200
vdot.t	S603,C630,C630
vexp2.p	C000,C100
vexp2.q	C000,C100
vexp2.s	S000,S100
vexp2.t	C000,C100
vf2h.q	C200,C100
vf2id.q	C200,C100,0
vf2in.q	C200,C100,0
vf2iu.q	C200,C100,0
vf2iz.q	C200,C100,0
vfad.p	S000,C100
vfad.q	S000,C100
vfad.t	S000,C100
vfim.s	S500,0.0111084
vfim.s	S501,-0.0111084
vfim.s	S502,0.0111084
vh2f.p	C200,C100
vhdp.p	S000,C100,C200
vhdp.q	S000,C100,C200
vhdp.t	S000,C100,C200
vhtfm3.t	R100,M700,R600
vhtfm4.q	R200,M700,R600
vi2c.q	S010,C000
vi2f.q	C010,C000,4
vi2f.q	C200,C100,0
vi2s.p	S010,C000
vi2s.q	C010,C000
vi2uc.q	S010,C000
vi2us.p	S010,C000
vi2us.q	C010,C000
vidt.q	R503
viim.s	S000,0
viim.s	S001,32767
viim.s	S001,90
viim.s	S002,-32768
viim.s	S003,0
viim.s	S010,1
viim.s	S011,-8
viim.s	S020,-3
viim.s	S021,-3
viim.s	S030,777
viim.s	S031,-1
viim.s	S130,0
vlog2.p	C000,C100
vlog2.q	C000,C100
vlog2.s	S000,S100
vlog2.t	C000,C100
vmax.p	C000,C100,C200
vmax.q	C000,C100,C200
vmax.s	S000,S100,S200
vmax.t	C000,C100,C200
vmidt.p	M000
vmidt.p	M022
vmidt.q	M000
vmidt.q	M700
vmidt.t	M000
vmidt.t	M600
vmin.p	C000,C100,C200
vmin.q	C000,C100,C200
vmin.s	S000,S100,S200
vmin.t	C000,C100,C200
vmmov.t	M700,M500
vmmul.p	E220,M020,M120
vmmul.p	M200,M000,M100
vmmul.p	M202,M002,E102
vmmul.p	M202,M002,M102
vmmul.p	M220,M020,M120
vmmul.p	M222,E022,M122
vmmul.p	M222,M022,M122
vmmul.q	M200,M000,M000
vmmul.q	M200,M000,M100
vmmul.q	M400,M600,M500
vmmul.t	M201,M000,M100
vmmul.t	M500,M700,M600
vmone.q	M000
vmov.p	C000,C100
vmov.p	R000,R000
vmov.p	R600,R500
vmov.p	R601,R500
vmov.q	C000,C000
vmov.q	C000,C100
vmov.q	C000,R000
vmov.q	C100,C100
vmov.q	C200,C200
vmov.q	C720,C730
vmov.q	R000,R000
vmov.q	R100,R100
vmov.s	S000,S100
vmov.s	S100,S100
vmov.t	C000,C100
vmov.t	R600,R500
vmov.t	R601,R500
vmov.t	R602,R500
vmscl.q	M100,E000,S200
vmul.p	C000,C100,C200
vmul.q	C000,C100,C200
vmul.q	R700,R701,R702
vmul.s	S000,S000,S000
vmul.s	S000,S000,S001
vmul.s	S000,S100,S200
vmul.s	S501,S502,S501
vmul.t	C000,C100,C200
vmul.t	R600,R600,C630
vmul.t	R601,R601,C630
vmul.t	R602,R602,C630
vmzero.q	M000
vmzero.q	M100
vmzero.q	M200
vmzero.q	M300
vmzero.q	M400
vmzero.q	M500
vmzero.q	M600
vmzero.q	M700
vmzero.t	M500
vneg.p	C000,C100
vneg.q	C000,C100
vneg.s	S000,S100
vneg.t	C000,C100
vnrcp.p	C000,C100
vnrcp.q	C000,C100
vnrcp.s	S000,S100
vnrcp.t	C000,C100
vnsin.p	C000,C100
vnsin.q	C000,C100
vnsin.s	S000,S100
vnsin.t	C000,C100
vocp.p	C000,C100
vocp.q	C000,C100
vocp.s	S000,S100
vocp.s	S603,S600
vocp.t	C000,C100
vone.p	C000
vone.q	C000
vone.s	S000
vone.t	C000
vpfxd	[-1:1],[-1:1],[-1:1],[-1:1]
vpfxd	[-1:1],[-1:1],[-1:1],m
vpfxd	[0:1],[0:1],[0:1],[0:1]
vpfxd	m,m,[0:1],m
vpfxd	m,m,m,m
vpfxs	-|x|,-|y|,-|z|,-|w|
vpfxs	-1/3,-1/2,-1/4,-1/6
vpfxs	-1/3,-2,-0,-3
vpfxs	-1/3,y,z,w
vpfxs	-3,1,2,3
vpfxs	-3,1/2,1/3,1/4
vpfxs	-w,-z,-y,-x
vpfxs	-x,-y,-z,-w
vpfxs	-y,0,x,w
vpfxs	-y,x,z,w
vpfxs	-z,-y,-y,-y
vpfxs	|x|,|y|,|z|,|w|
vpfxs	|z|,-y,-y,-y
vpfxs	0,-y,x,w
vpfxs	0,0,0,0
vpfxs	0,1,2,1/2
vpfxs	0,1,2,3
vpfxs	0,1/2,1/3,1/4
vpfxs	0,x,-y,w
vpfxs	0,x,y,w
vpfxs	0,y,x,w
vpfxs	1,1,1,1
vpfxs	1,y,z,w
vpfxs	3,1/3,1/4,1/6
vpfxs	3,2,1,0
vpfxs	3,3,3,3
vpfxs	w,w,w,w
vpfxs	w,y,z,x
vpfxs	x,-y,z,w
vpfxs	x,0,-y,w
vpfxs	x,0,y,w
vpfxs	x,x,x,x
vpfxs	x,y,y,w
vpfxs	x,y,z,w
vpfxs	y,-1,z,-y
vpfxs	y,-y,z,-z
vpfxs	y,0,x,w
vpfxs	y,x,y,w
vpfxs	y,x,z,w
vpfxs	y,y,x,w
vpfxs	y,y,y,y
vpfxs	z,z,z,z
vpfxt	-y,x,1,w
vpfxt	1,-z,y,w
vpfxt	1/2,y,z,w
vpfxt	1/3,|y|,|z|,|x|
vpfxt	3,-w,-w,-w
vpfxt	z,1,-x,w
vqmul.q	C000,C100,C200
vrcp.p	C000,C100
vrcp.q	C000,C100
vrcp.s	S000,S100
vrcp.s	S001,S001
vrcp.t	C000,C100
vrexp2.p	C000,C100
vrexp2.q	C000,C100
vrexp2.s	S000,S100
vrexp2.t	C000,C100
vrot.p	R500,S501,[c,s]
vrot.q	R000,S601,[c,s,s,s]
vrot.q	R001,S601,[s,c,0,0]
vrot.q	R002,S601,[s,0,c,0]
vrot.q	R003,S601,[s,0,0,c]
vrot.q	R100,S601,[c,s,0,0]
vrot.q	R101,S601,[s,c,s,s]
vrot.q	R102,S601,[0,s,c,0]
vrot.q	R103,S601,[0,s,0,c]
vrot.q	R200,S601,[c,0,s,0]
vrot.q	R201,S601,[0,c,s,0]
vrot.q	R202,S601,[s,s,c,s]
vrot.q	R203,S601,[0,0,s,c]
vrot.q	R300,S601,[c,0,0,s]
vrot.q	R301,S601,[0,c,0,s]
vrot.q	R302,S601,[0,0,c,s]
vrot.q	R303,S601,[-s,-s,-s,c]
vrot.q	R400,S601,[c,-s,-s,-s]
vrot.q	R401,S601,[-s,c,0,0]
vrot.q	R402,S601,[-s,0,c,0]
vrot.q	R403,S601,[-s,0,0,c]
vrot.q	R500,S601,[c,-s,0,0]
vrot.q	R501,S601,[-s,c,-s,-s]
vrot.q	R502,S601,[0,-s,c,0]
vrot.q	R503,S601,[0,-s,0,c]
vrot.q	R600,S501,[c,0,-s,0]
vrot.q	R601,S501,[0,c,-s,0]
vrot.q	R602,S501,[-s,-s,c,-s]
vrot.q	R603,S501,[0,0,-s,c]
vrot.q	R700,S501,[c,0,0,-s]
vrot.q	R701,S501,[0,c,0,-s]
vrot.q	R702,S501,[0,0,c,-s]
vrot.q	R703,S501,[-s,-s,-s,c]
vrsq.p	C000,C100
vrsq.q	C000,C100
vrsq.s	S000,S100
vrsq.s	S603,S603
vrsq.t	C000,C100
vs2i.p	C010,C000
vs2i.s	C010,S000
vsat0.p	C000,C100
vsat0.q	C000,C100
vsat0.s	S000,S100
vsat0.t	C000,C100
vsat1.p	C000,C100
vsat1.q	C000,C100
vsat1.s	S000,S100
vsat1.t	C000,C100
vsbn.s	S010,S000,S100
vsbn.s	S011,S000,S101
vsbn.s	S012,S000,S102
vsbn.s	S013,S000,S103
vsbn.s	S020,S000,S110
vsbn.s	S021,S000,S111
vsbn.s	S022,S000,S112
vsbn.s	S023,S000,S113
vscl.p	C000,C100,S200
vscl.q	C000,C100,S200
vscl.q	R000,R000,S100
vscl.t	C000,C100,S200
vscl.t	C630,C630,S603
vscl.t	R500,R503,S630
vscl.t	R501,R503,S631
vscl.t	R502,R503,S632
vscl.t	R503,C630,S603
vscmp.p	C000,C100,C200
vscmp.q	C000,C100,C200
vscmp.s	S000,S100,S200
vscmp.t	C000,C100,C200
vsge.p	C000,C100,C200
vsge.q	C000,C100,C200
vsge.s	S000,S100,S200
vsge.t	C000,C100,C200
vsgn.p	C000,C100
vsgn.q	C000,C100
vsgn.s	S000,S100
vsgn.t	C000,C100
vsin.p	C000,C100
vsin.q	C000,C100
vsin.s	S000,S100
vsin.t	C000,C100
vslt.p	C000,C100,C200
vslt.q	C000,C100,C200
vslt.s	S000,S100,S200
vslt.t	C000,C100,C200
vsocp.p	C000,C100
vsocp.s	C000,S100
vsqrt.p	C000,C100
vsqrt.q	C000,C100
vsqrt.s	S000,S100
vsqrt.t	C000,C100
vsrt1.q	C000,C100
vsrt2.q	C000,C100
vsrt3.q	C000,C100
vsrt4.q	C000,C100
vsub.p	C000,C100,C200
vsub.q	C000,C100,C200
vsub.s	S000,S100,S200
vsub.t	C000,C100,C200
vt4444.q	C010,C000
vt5551.q	C010,C000
vt5650.q	C010,C000
vtfm3.t	R100,M700,R600
vtfm4.q	R200,M700,R600
vus2i.p	C010,C000
vus2i.s	C010,S000
vwbn.s	S000,S000,0x00
vwbn.s	S001,S000,0x08
vwbn.s	S002,S000,0x10
vwbn.s	S003,S000,0x20
vwbn.s	S010,S000,0x40
vwbn.s	S011,S000,0x80
vwbn.s	S012,S000,0x90
vwbn.s	S013,S000,0xc0
vwbn.s	S020,S000,0x7d
vwbn.s	S021,S000,0x7e
vwbn.s	S022,S000,0x7f
vwbn.s	S023,S000,0x80
vwbn.s	S030,S000,0x82
vwbn.s	S031,S000,0x90
vwbn.s	S032,S000,0xc0
vwbn.s	S033,S000,0xff
vzero.p	C000
vzero.q	C000
vzero.s	S000
vzero.t	C000

.close
