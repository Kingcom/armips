.gba
.create output.bin,0

EQUTEST equ "test"

; check if only whole words are replaced
OTHEREQUTEST equ ".test"

.ascii ' ',EQUTEST,'!'
.ascii OTHERQUTEST

.close