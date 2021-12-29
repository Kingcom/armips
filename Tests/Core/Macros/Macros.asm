.gba
.create "output.bin",0

; check token replacement
.macro pushWrapper, registers
  push registers
.endmacro

.macro ldrWrapper, target, source
  ldr target,source
.endmacro

pushWrapper {r5,r7}
ldrWrapper r2,[r4, r5]

; check parameters containing function calls
.macro cat, a, b
  .byte "" + a + b
  .align
.endmacro

cat toHex(255, 4), toHex(256, 3)

; check equ usage in macros
.macro print, a
  value equ "<" + a + ">"
  .notice value
.endmacro

print "test1"
print "test2"

.close
