.gba
.create "output.bin",0

.relativeinclude off

.include "a.asm"
.include "sub/a.asm"

.relativeinclude on
.include "a.asm"
.include "sub/a.asm"

.include "sub/sub.asm"

.include "█.asm"

.close
