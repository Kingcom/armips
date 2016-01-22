.gba
.create "output.bin",0

.macro test0,name
	.notice °name + "(): "
		+ name()
.endmacro

.macro test1,name,a
	.notice °name + "(" + °a + "): "
		+ name(a)
.endmacro

.macro test1h,name,a
	.notice °name + "(" + °a + "): "
		+ toHex(name(a))
.endmacro

.macro test2,name,a,b
	.notice °name + "(" + °a + "," + °b + "): "
		+ name(a,b)
.endmacro

.macro test2h,name,a,b
	.notice °name + "(" + °a + "," + °b + "): "
		+ toHex(name(a,b))
.endmacro

.macro test3,name,a,b,c
	.notice °(name) + "(" + °(a) + "," + °(b) + "," + °(c) + "): "
		+ name(a,b,c)
.endmacro

fileA equ "file.bin"
fileB equ "file.dat"

test0	endianness

test1	toString,191
test2	toHex,191,6

test1	fileExists,fileA
test1	fileExists,fileB
test1	fileSize,fileA

test2h	readU8,fileA,2
test2h	readU16,fileA,2
test1h	readU32,fileA


str equ "teststest"
part equ "test"

test1	strlen,str
test3	substr,str,0,4
test3	find,str,part,0
test3	find,str,part,1
test2	rfind,str,part

label:
test1	defined,label
test1	defined,label1

.close
