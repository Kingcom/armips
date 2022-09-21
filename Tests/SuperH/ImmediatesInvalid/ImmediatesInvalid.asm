.saturn
.create "output.bin",0

    mov.l OutofRangeValue32,r0 ;; 0
    mov.w OutofRangeValue16,r0 ;; 2

.org 0x206
OutofRangeValue16:
    .dh 0x1234
.org 0x404
OutofRangeValue32:
    .dw 0x12345678

.close
