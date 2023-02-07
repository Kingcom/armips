.saturn
.create "output.bin",0

BackwardsBranching:
    mov #0x00,r0  ; min value, min register
    mov #0xFF,r0  ; max value, min register

    mov #0x00,r15 ; min value, max register
    mov #0xFF,r15 ; max value, max register

    mov #-1,r0
    mov #-127,r0

    mov #-1,r15
    mov #-127,r15

    bt ForwardBranching

    mov.l Some32BitValue, r0
    mov.w Some16BitValue, r0
    mova Some16BitValue,r0
    mova Some32BitValue,r0

    bf BackwardsBranching

    add #0x00,r0
    add #0xFF,r0

    add #0x00,r15
    add #0xFF,r15

    add #-1,r0
    add #-127,r0

    add #-1,r15
    add #-127,r15

ForwardBranching:

    cmp/eq #0x00,r0
    cmp/eq #0xFF,r0

    cmp/eq #-1,r0
    cmp/eq #-127,r0

    ; PC relative loads in different positions
    mova Some16BitValue,r0
    mova Some32BitValue+4,r0
    mov.l Some32BitValue, r15
    mov.w Some16BitValue+2, r15

    and.b #0x00,@(r0,gbr)
    and.b #0xFF,@(r0,gbr)
    or.b  #0x00,@(r0,gbr)
    or.b  #0xFF,@(r0,gbr)
    tst.b #0x00,@(r0,gbr)
    tst.b #0xFF,@(r0,gbr)
    xor.b #0x00,@(r0,gbr)
    xor.b #0xFF,@(r0,gbr)
    trapa #0x00
    trapa #0xFF

    .align 4

Some32BitValue: .dd 0x12345678, 0x87654321
Some16BitValue: .dw 0x1234, 0x5678

.close
