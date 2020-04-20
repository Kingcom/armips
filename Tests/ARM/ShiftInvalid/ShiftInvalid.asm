.gba
.create "output.bin",0

.thumb
	lsl	r1,r2,32	// Invalid
	asl	r1,r2,32	// Invalid (same as lsl)
	lsr	r1,r2,33	// Invalid
	asr	r1,r2,33	// Invalid

.arm
	mov	r1,r2,lsl 32	// Invalid
	mov	r1,r2,asl 32	// Invalid (alias for lsl)
	mov	r1,r2,lsr 33	// Invalid
	mov	r1,r2,asr 33	// Invalid

.close
