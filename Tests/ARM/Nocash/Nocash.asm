.gba
.create "output.bin",0

.arm

// Faux GBA ROM header
// This way we can load the binary in No$gba for verification
.area 0xC0,0x00
b	armMsg
.endarea

.arm

.align 4
armMsg:

// Example from No$gba help file
.msg	"Hello"
.msg	"Source Addr = %r0% ; Dest Addr = %r1% ; Length = %r2%"
.msg	"Stack Pointer = %SP%"
.msg	"Decompression time: %lastclks% cycles"

// Test empty message
// Empty message should output at least one null byte,
// otherwise No$gba cannot parse it correctly.
.msg	""

// Test message alignment
.msg	"1"
.msg	"12"
.msg	"123"
.msg	"1234"

// Test maximum possible length (120)
.msg	"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

// Jump to THUMB mode
add	r0,=thumbMsg
add	r0,0x1
bx	r0

.thumb

.align 4 // so we can use add r,=
thumbMsg:

// Example from No$gba help file
.msg	"Hello"
.msg	"Source Addr = %r0% ; Dest Addr = %r1% ; Length = %r2%"
.msg	"Stack Pointer = %SP%"
.msg	"Decompression time: %lastclks% cycles"

// Test empty message
// Empty message should output at least one null byte,
// otherwise No$gba cannot parse it correctly.
.msg	""

// Test message alignment
.msg	"A"
.msg	"AB"
.msg	"ABC"
.msg	"ABCD"

// Test maximum possible length (120)
// Note: No$gba's debug window may bug out when scrolling this message into view
.msg	"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

b	.

.close
