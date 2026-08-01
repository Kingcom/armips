.n64
.create "output.bin",0

addiu $1, $1, -0x8000
addiu $1, $1, 0x7FFF
andi $1, $1, 0xFFFF
sll $1, $1, 31

.relaximmsign on
addiu $1, $1, 0xFFFF
andi $1, $1, -1
sll $1, $1, -1
.relaximmsign off

.close
