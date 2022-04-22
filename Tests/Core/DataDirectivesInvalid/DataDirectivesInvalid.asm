.gba
.create "output.bin",0

// Test .ascii with non-ASCII character
.ascii	"é"
.asciiz	"é"

// Test basic data directives with non-ASCII character
.byte		"é"
.halfword	"é"
.word		"é"
.doubleword	"é"
.db		"é"
.dh		"é"
.dw		"é"
.dd		"é"
.dcb		"é"
.dcw		"é"
.dcd		"é"
.dcq		"é"
db		"é"
dh		"é"
dw		"é"
dd		"é"
dcb		"é"
dcw		"é"
dcd		"é"
dcq		"é"

.close
