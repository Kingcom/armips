.rsp
.create "output.bin", 0

; RSP opcodes

mfc2 r1,v1[e15]
mtc2 r1,v1[e15]
cfc2 r1,$2
ctc2 r1,$2

vmulf v1,v1,v1[e15]
vmulu v1,v1,v1[e15]
vrndp v1,v1,v1[e15]
vmulq v1,v1,v1[e15]
vmudl v1,v1,v1[e15]
vmudm v1,v1,v1[e15]
vmudn v1,v1,v1[e15]
vmudh v1,v1,v1[e15]

vmacf v1,v1,v1[e15]
vmacu v1,v1,v1[e15]
vrndn v1,v1,v1[e15]
vmacq v1,v1,v1[e15]
vmadl v1,v1,v1[e15]
vmadm v1,v1,v1[e15]
vmadn v1,v1,v1[e15]
vmadh v1,v1,v1[e15]

vadd v1,v1,v1[e15]
vsub v1,v1,v1[e15]
vsut v1,v1,v1[e15]
vabs v1,v1,v1[e15]
vaddc v1,v1,v1[e15]
vsubc v1,v1,v1[e15]
vaddb v1,v1,v1[e15]
vsubb v1,v1,v1[e15]
vaccb v1,v1,v1[e15]
vsucb v1,v1,v1[e15]
vsad v1,v1,v1[e15]
vsac v1,v1,v1[e15]
vsum v1,v1,v1[e15]
vsar v1,v1,v1[e15]
vacc v1,v1,v1[e15]
vsuc v1,v1,v1[e15]

vlt v1,v1,v1[e15]
veq v1,v1,v1[e15]
vne v1,v1,v1[e15]
vge v1,v1,v1[e15]
vcl v1,v1,v1[e15]
vch v1,v1,v1[e15]
vcr v1,v1,v1[e15]
vmrg v1,v1,v1[e15]

vand v1,v1,v1[e15]
vnand v1,v1,v1[e15]
vor v1,v1,v1[e15]
vnor v1,v1,v1[e15]
vxor v1,v1,v1[e15]
vnxor v1,v1,v1[e15]

vrcp v1[e1],v1[e15]
vrcpl v1[e1],v1[e15]
vrcph v1[e1],v1[e15]
vmov v1[e1],v1[e15]
vrsq v1[e1],v1[e15]
vrsql v1[e1],v1[e15]
vrsqh v1[e1],v1[e15]
vnop

vextt v1,v1,v1[e15]
vextq v1,v1,v1[e15]
vextn v1,v1,v1[e15]

vinst v1,v1,v1[e15]
vinsq v1,v1,v1[e15]
vinsn v1,v1,v1[e15]
vnull

lbv v1[e15],0x3F(r31)
lsv v1[e15],0x3F(r31)
llv v1[e15],0x3F(r31)
ldv v1[e15],0x3F(r31)
lqv v1[e15],0x3F(r31)
lrv v1[e15],0x3F(r31)
lpv v1[e15],0x3F(r31)
luv v1[e15],0x3F(r31)
lhv v1[e15],0x3F(r31)
lfv v1[e15],0x3F(r31)
lwv v1[e15],0x3F(r31)
ltv v1[e15],0x3F(r31)

sbv v1[e15],0x3F(r31)
ssv v1[e15],0x3F(r31)
slv v1[e15],0x3F(r31)
sdv v1[e15],0x3F(r31)
sqv v1[e15],0x3F(r31)
srv v1[e15],0x3F(r31)
spv v1[e15],0x3F(r31)
suv v1[e15],0x3F(r31)
shv v1[e15],0x3F(r31)
sfv v1[e15],0x3F(r31)
swv v1[e15],0x3F(r31)
stv v1[e15],0x3F(r31)

.close
