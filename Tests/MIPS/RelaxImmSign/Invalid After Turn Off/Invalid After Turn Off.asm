.n64
.create "output.bin",0

.relaximmsign on
addiu $1, $1, 0xFFFF
.relaximmsign off
addiu $1, $1, 0xFFFF

.close
