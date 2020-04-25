.gba
.create "output.bin",0x1000

.org 0x1100

label1:

.notice "org(label1): " + toHex(org(label1), 16)
.notice "orga(label1): " + toHex(orga(label1), 16)
.notice "headersize(label1): " + toHex(headersize(label1), 16)

.org 0x2800

.notice "org(): " + toHex(org(), 16)
.notice "orga(): " + toHex(orga(), 16)
.notice "headersize(): " + toHex(headersize(), 16)

.close
