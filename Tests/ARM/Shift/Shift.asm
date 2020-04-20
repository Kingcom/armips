.gba
.create "output.bin",0


.thumb

	lsl	r1,r2,0		// Equivalent to mov r1,r2
	lsl	r1,r2,1
	lsl	r1,r2,31
//	lsl	r1,r2,32	// Invalid

	// asl (alias for lsl)
	asl	r1,r2,0		// Equivalent to mov r1,r2
	asl	r1,r2,1
	asl	r1,r2,31
//	asl	r1,r2,32	// Invalid

	lsr	r1,r2,0		// Converted to lsl 0
	lsr	r1,r2,1
	lsr	r1,r2,31
	lsr	r1,r2,32	// Encoded as lsr 0

	asr	r1,r2,0		// Converted to lsl 0
	asr	r1,r2,1
	asr	r1,r2,31
	asr	r1,r2,32	// Encoded as asr 0


.arm

	mov	r1,r2,lsl 0	// Equal to mov r1,r2
	mov	r1,r2,lsl 1
	mov	r1,r2,lsl 31
//	mov	r1,r2,lsl 32	// Invalid

	// asl (alias for lsl)
	mov	r1,r2,asl 0	// Equal to mov r1,r2
	mov	r1,r2,asl 1
	mov	r1,r2,asl 31
//	mov	r1,r2,lsl 32	// Invalid

	mov	r1,r2,lsr 0	// Converted to lsl 0
	mov	r1,r2,lsr 1
	mov	r1,r2,lsr 31
	mov	r1,r2,lsr 32	// Encoded as lsr 0

	mov	r1,r2,asr 0	// Converted to lsl 0
	mov	r1,r2,asr 1
	mov	r1,r2,asr 31
	mov	r1,r2,asr 32	// Encoded as asr 0


.close

