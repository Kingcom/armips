.gba
.create "output.bin", 0

.include "shift_jis.asm", "Shift-JIS"
.include "utf8.asm"
.include "utf8_bom.asm"
.include "utf16_be.asm"
.include "utf16_le.asm"

.close