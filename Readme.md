ARMIPS assembler v0.9  
by Kingcom  
https://github.com/Kingcom/armips
http://buildbot.orphis.net/armips/

# 1. Introduction

Note: This file is still incomplete, some information is missing or may be outdated.

## 1.1 Usage

The assembler is called from the command line. There is both an x86 and an x86-64 version. Depending on the version, the usage is as follows:

```
  armips code.asm [optional parameters]
  armips64 code.asm [optional parameters]
```

Code.asm is the main file of your assembly code, which can open and include other files.  
The following optional command line parameters are supported:

* -temp fileName
        Specifies the output name for temporary assembly data.

* -sym fileName
        Specifies the output name for symbol data in the sym format.

* -sym2 fileName
        Specifies the output name for symbol data in the sym2 format.

* -erroronwarning
        Specifies that any warnings shall be treated like errors, preventing assembling. This has the same effect as the ```.erroronwarning``` directive.

* -equ name replacement
        Equivalent to using ```name equ replacement``` in the assembly code.

* -strequ name replacement
        Equivalent to using ```name equ "replacement"``` in the assembly code.

* -root folderName
        Specifies the folder to be used as the working directory during execution.

The optional -temp parameter specifies a text file where all the temporary assembly data will be written to for reference purposes. It will look like this:

```
  ; 1 file  included
  ; test.asm

  00000000 .open "SLPM_870.50",0x8000F800  ; test.asm line 1
  8000F800 .org 0x800362DC                 ; test.asm line 5
  800362DC   jal     0x801EBA3C            ; test.asm line 7
  800362E0 .Close                          ; test.asm line 9
```

## 1.2  Change log

* Version 0.9
    * huge rewrite with many enhancements and fixes
    * can now read from UTF8, UTF16, and Shift-JIS files and convert the input correctly
    * PSP support, load ELFs with ```.loadelf```
    * able to import and relocate static C/C++ libraries
    * new ```-sym2``` format for use with PPSSPP and PCSX2
    * new directives: ```.sym```, ```.stringn```, ```.sjis```, ```.sjisn```, ```.function```, ```.endfunction```, ```.importlib```, ```.loadelf```, ...
    * removed directives: ```.ifarm```, ```.ifthumb```, ```.radix```
* Version 0.7d
    * added automatic optimizations for several ARM opcodes
    * many bugfixes and internal changes
    * added static labels
    * new directives: ```.warning```, ```.error```, ```.notice```, ```.relativeinclude```, ```.erroronwarning```, ```.ifarm```, ```.ifthumb```
    * quotation marks can now be escaped in strings using ```\"```.
* Version 0.7c
    * Macros can now contain unique local labels
    * ```.area``` directive added
    * countless bugfixes
    * no$gba debug message support
    * full no$gba sym support
* Version 0.7b
    * ARM/THUMB support
    * fixed break/syscall MIPS opcodes 
    * added check if a MIPS instruction is valid inside a delay slot
    * fixed and extended base detection
    * added "." dummy label to the math parser to get the current memory address
    * added ```dcb```/```dcw```/```dcd``` directives

* Version 0.5b
    * Initial release

	
## 1.3  Migration from older versions

There are several changes after version 0.7d that may break compatibility with code written for older versions. These are as follows:

* String literals now require quotation marks, e.g. for file names
* $XX is no longer supported for hexadecimal literals

# 2. Building from source

You can find the latest code at the [GitHub repository](https://github.com/Kingcom/armips). Make sure to initialize and update submodules, too.

Build instructions per platform:
* Building on Windows: You will need Visual Studio 2015 (Community Edition is sufficient). Simply open armips.sln and compile it.
* Building on Unix: You will need CMake and a C++11 compliant compiler. Run CMake on the root directory of the repository, then simply run make.

# 3. General information

The assembler includes full support for the MIPS R3000, MIPS R4000, and Allegrex instruction sets, partial support for the EmotionEngine instruction set, as well as complete support for the ARM7 and ARM9 instruction sets, both THUMB and ARM mode. Among the other features of the assembler are:

* a full fledged C-like math parser. It should behave exactly like in any C/C++ code, including all the weirdness. All immediate values can be specified by an expression, though some directives can't use variable addresses including labels
* you can open several files in a row, but only one output file can be open at any time. You can specify its address in memory to allow overlay support. Any file can cross-reference any other included file
* local, static, and global labels (see 3.3)
* table support for user defined text encodings (see 3.7)
* several MIPS macros to make writing code easier and faster (see 4.1)
* user defined macros (see 5.3)
* built-in checks for possible load delay problems (see 3.6)
* optional automatic fix for said problems by inserting a nop between the instructions
* output of the assembled code to a text file, with memory addresses and origin (see 1.2)
* a directive to ensure that data is not bigger than a user defined size (see 3.8)

# 3. Features

## 3.1  Files

Unlike other assemblers, you don't specify the input/output file as a command line argument. You have to open the file in the source code, and also close it yourself. This was done in order to support overlays, which are very common in PSX and NDS games. Instead of only having one output file, you can have as many as you need - each with its own address in memory. The files can cross-reference each other without any problems, so you can call code from other files that are currently not opened as well.

```
  .Open "SLPS_035.71", 0x8000F800
  ; ...
  .Close
  .Open "System\0007.dat", 0x800CC000
  ; ...
  .Close
```

## 3.2  Comments

Both ```;``` and ```//``` are supported for comments.

## 3.3  Labels

There is support for both local, global and static labels. Local labels are only valid in the area between the previous and the next global label. Specific directives, like .org, will also terminate the area. A label is defined by writing a colon after its name. All labels can be used before they are defined.

```
  GlobalLabel:      ; This is a global label
  @@LocalLabel:     ; This is a local label, it is only
                    ; valid until the next global one
  OtherGlobalLabel: ; this will terminate the area where
                    ; @@LocalLabel can be used
  b   @@LocalLabel  ; as a result, this will cause an error
```

Static labels behave like global labels, but are only valid in the very file they were defined. Any included files or files that include it cannot reference it. They can, however, contain another static label with the same name.

```
  @staticlabel:
```

A label name can contain all characters from A-Z, numbers, and underscores. However, it cannot start with a digit. All label names are case insensitive.

Additionally, ```.``` can be used to reference the current memory address.


## 3.4 equ

This works as a text replacement and is defined as follows:

```
  @@StringPointer equ 0x20(r29)
```

There has to be a space before and after equ. The assembler will replace any occurance of ```@@StringPointer``` with ```0x20(r29)```. As it is a local equ, it will only do so in the current section, which is terminated by any global label or specific directives. This code:

```
  @@StringPointer equ 0x20(r29)
 
  lw  a0,@@StringPointer
  nop
  sw  a1,@@StringPointer
```

will assemble to this:

```
  lw  a0,0x20(r29)
  nop
  sw  a1,0x20(r29)
```

There can be both global and local equs, but unlike normal labels, they have to be defined before they are used.


## 3.5 Math Parser

A standard math parser with operator precedence and bracket support has been implemented. It is intended to behave exactly like any C/C++ parser and supports all unary, binary and tertiary operators of the C language. Every numeral argument can be given as an expression, including label names. However, some directives do not support variable addresses, so labels can not be used in expressions for them. The following bases are supported:

* 0xA and 0Ah for hexadecimal numbers
* 0o12 and 12o for octal numbers
* 1010b and 0b1010 for binary numbers

Everything else is interpreted as a decimal numbers, so a leading zero does not indicate an octal number. Be aware that every number has to actually start with a digit, unless you use $ for hexadecimal numbers. For example, as FFh is a perfectly valid label name, you have to write 0FFh in this case. Labels, on the other hand, cannot start with a digit.

A few examples:

```
  mov  r0,10+0xA+$A+0Ah+0o12+12o+1010b
  ldr  r1,=ThumbFunction+1
  li   v0,Structure+(3*StructureSize)
```

## 3.6 Load delay detection

This feature is still unfinished and experimental. It works in most cases, though. On certain MIPS platforms (most notably the PlayStation 1), any load is asynchronously delayed by one cycle and the CPU won't stall if you attempt to use it before. Attempts to use it will return the old value on an actual system (emulators usually do not emulate this, which makes spotting these mistakes even more difficult). Therefore, the assembler will attempt to detect when such a case happens. The following code would result in a warning:

```
  lw   a0,0x10(r29)
  lbu  a1,(a0)
```

This code doesn't take the load delay into account and will therefore only work on emulators. The assembler detects it and warns the user. In order to work correctly, the code should look like this:

```
  lw   a0,0x10(r29)
  nop
  lbu  a1,(a0)
```

The assembler can optionally automatically insert a nop when it detects such an issue. This can be enabled with the ```.fixloaddelay``` directive.  
However, as there is no control flow analysis, there is a chance of false positives. For example, a branch delay slot may cause a warning for the the opcode that follows it, even if there is no chance that they will be executed sequentially. The following example illustrates this:

```
  bnez  a0,@@branch1
  nop
  j     @@branch2
  lw    a0,(a1)
@@branch1:
  lbu   a2,(a0)
```

You can fix the false warning by using the ```.resetdelay``` directive before the last instruction.

```
  bnez  a0,@@branch1
  nop
  j     @@branch2
  lw    a0,(a1)
.resetdelay
@@branch1:
  lbu   a2,(a0)
```

## 3.7 Strings

You can write ASCII text by simple using the ```.db```/```.ascii``` directive. However, you can also write text with custom encodings. In order to do that, you first have to load a table, and then use the .string directive to write the text. It behaves exactly like the ```.db``` instruction (so you can also specify immediate values as arguments), with the exception that it uses the table to encode the text, and appends a termination sequence after the last argument. This has to be specified inside the table, otherwise 0 is used.

```
  .loadtable "custom.tbl"
  .string "Custom text",0xA,"and more."
```

The first and third argument are encoded according to the table, while the second one is written as-is.

Quotation marks can be escaped by prefixing them with a backslash. Any backlash not followed by a quotation mark is kept as-is. If you want to use a backslash at the end of a string, prefix it by another backlash.  
For example, to write a quotation mark followed by a backlash:

```
   .ascii "\"\\"
```

## 3.8 Areas

If you overwrite existing data, it is critical that you don't overwrite too much. The area directive will take care of checking if all the data is within a given space. In order to do that, you just have to specify the maximum size allowed.

```
.area 10h
  .word 1,2,3,4,5
.endarea
```

This would cause an error on assembling, because the word directive takes up 20 bytes instead of the 16 that the area is allowed to have. This, on the other hand, would assemble without problems:

```
.org 8000000h
.area 8000020h-.
  .word 1,2,3,4,5
.endarea
```

Here, the area is 32 bytes, which is sufficient for the 20 bytes used by .word.  
Optionally, a second parameter can be given. The remaining free size of the area will then be completely filled with bytes of that value.

## 3.9 Symbol files

Functions.

## 3.10 C/C++ importer

You can link object files or static libraries in ELF format. The code and data is relocated to the current output position and all of its symbols are exported. You can in turn use armips symbols inside of your compiled code by declaring them as extern. Note: As armips labels are case insensitive, the exported symbols are treated the same way. Be aware of name mangling when trying to reference C++ functions, and consider declaring them as ```extern "C"```.

You can optionally supply names for constructor and destructor functions. Functions with those names will be generated that call of the global constructors/destructors of the imported files.

```
	.importlib "code.a",globalConstructor,globalDestructor
```

# 4. Assembler directives

These commands tell the assembler to do various things like opening the output file or opening another source file.

## 4.1  General Directives

### Setting the architecture

These directives can be used to set the architecture that the following assembly code should be parsed and output for. The architecture can be changed at any time without affecting the preceding code.

| Directive | System | Architecture | Comment |
| --------- |:------:|:------------:|:--------|
| ```.psx``` | PlayStation 1 | MIPS R3000 | - |
| ```.ps2``` | PlayStation 2 | EmotionEngine | - |
| ```.psp``` | PlayStation Portable | Allegrex | - |
| ```.n64``` | Nintendo 64 | MIPS R4000 | - |
| ```.rsp``` | Nintendo 64 | RSP | - |
| ```.gba``` | GameBoy Advance | ARM7 | Defaults to THUMB mode |
| ```.nds``` | Nintendo DS | ARM9 | Defaults to ARM mode |
| ```.3ds``` | Nintendo 3DS | ARM11 | Defaults to ARM mode, incomplete |
| ```.arm.big``` | - | ARM | Output in big endian |
| ```.arm.little``` | - | ARM | Output in little endian |

### Opening a generic file

```
.open FileName,RamAddress  
.open OldFileName,NewFileName,MemoryAddress
```

Opens the specified file for output. If two file names are specified, then the assembler will copy the file specified by the file name to the second path. If relative include is off, all paths are relative to the current working directory. Otherwise the path is relative to the including assembly file. MemoryAddress specifies the difference between the first byte of the file and its position in memory. So if file position 0x800 is at position 0x80010000 in memory, the header size is 0x80010000-0x800=0x8000F800. It can be changed later with the ```.headersize``` directive.  
Only the changes specified by the assembly code will be inserted, the rest of the file remains untouched.

### Creating a new file

```
.create FileName,MemoryAddress
.createfile FileName,MemoryAddress
```

Creates the specified file for output. If the file already exists, it will be overwritten. If relative include is off, all paths are relative to the current working directory. Otherwise the path is relative to the including assembly file. MemoryAddress specifies the difference between the first byte of the file and its position in memory. So if file position 0x800 is at position 0x80010000 in memory, the header size is 0x80010000-0x800=0x8000F800. It can be changed later with the [```.headersize```](#changing-the-header-size) directive.

### Closing a file

```
.close
.closefile
```

Closes the currently opened output file.

### Setting the output position

```
.org RamAddress
.orga FileAddress
```

Sets the output pointer to the specified address. ```.org``` specifies a memory address, which is automatically converted to the file address for the current output file. ```.orga``` directly specifies the file address.

### Changing the header size


### Including another assembly file

```
.include FileName[,encoding]
```

Opens the file called FileName to assemble its content. If relative include is off, all paths are relative to the current working directory. Otherwise the path is relative to the including assembly file. You can include other files up to a depth level of 64. This limit was added to prevent the assembler from getting stuck in an infinite loop due to two files including each other recursively. If the included file has an Unicode Byte Order Mark then the encoding will be automatically detected. If no Byte Order Mark is present it will default to UTF-8. This can be overwritten by manually specifying the file encoding as a second parameter.

The following values  are supported:
* SJIS, Shift-JIS
* UTF8, UTF-8
* UTF16, UTF-16, UTF16-BE, UTF-16-BE
* ASCII

## Text and data directives

### Aligning the output position

```
.align num
```

Writes zeros into the output file until the output position is a multiple of ```num```. ```num``` has to be a power of two. 

### Filling space with a value

```
.fill length[,value]
defs length[,value]
```

Inserts ```length``` amount of bytes of ```value```. If ```value``` isn't specified, zeros are inserted. Only the lowest 8 bits of ```value``` are inserted.

### Including a binary file

```
.incbin FileName[,start[,size]]
.import FileName[,start[,size]]
```

Inserts the file specified by FileName into the currently opened output file. If relative include is off, all paths are relative to the current working directory. Otherwise the path is relative to the including assembly file. Optionally, start can specify the start position in the file from it should be imported, and size can specify the number of bytes to read.

### Write bytes

```
.byte value[,...]
.db value[,...]
.ascii value[,...]
dcb value[,...]
```

Inserts the specified sequence of bytes. Each parameter can be any expression that evaluates to an integer or a string. If it evaluates to an integer, only the lowest 8 bits are inserted. If it evaluates to a string, every character is inserted as a byte. 

### Write halfwords

```
.halfword value[,...]
.dh value[,...]
dcw value[,...]
```

Inserts the specified sequence of halfwords. Each parameter can be any expression that evaluates to an integer or a string. If it evaluates to an integer, only the lowest 16 bits are inserted. If it evaluates to a string, every character is inserted as a halfword. 


### Write words

```
.word value[,...]
.dw value[,...]
dcd value[,...]
```

Inserts the specified sequence of halfwords. Each parameter can be any expression that evaluates to an integer, a string, or a floating point number. If it evaluates to an integer, only the lowest 32 bits are inserted. If it evaluates to a string, every character is inserted as a word. Floats are inserted using an integer representation of the float's encoding

### Write floating point numbers

```
.float value[,...]
```

Inserts the specified sequence of single precision floats. Each parameter can be any expression that evaluates to an integer or a floating point number. If it evaluates to an integer, it will be converted to a floating point number of that value.


### Load a table specifying a custom encoding

```
.loadtable TableName[,encoding]
.table TableName[,encoding]
```

Loads ```TableName``` for using it with the .string directive. The encoding can be specified in the same way as for ```.include```. You can specify one or several termination bytes like this:

```
	02=a
	/FF
```

FF will be used as the termination sequence. If it is not given, zero is used instead. Strings are matched using the longest prefix found in the table.


### Write text with custom encoding

```
.string "String"[,...]
.stringn "String"[,...]
.str "String"[,...]
.strn "String"[,...]
```

Inserts the given string using the encoding from the currently loaded table. ```.string``` and ```.str``` insert the termination sequence specified by the table after the string, while ```.stringn``` and ```.strn``` omit it.

### Write text with Shift-JIS encoding

```
.sjis "String"[,...]
.sjisn "String"[,...]
```

Inserts the given string using the Shift-JIS encoding. ```.sjis``` inserts a null byte after the string, while ```.sjisn``` omits it.

## Conditional directives

### Beginning a conditional block

```
.if cond
.ifdef identifier
.ifndef idenifier
```

The content of a conditional block will only be used if the condition is met. In the case of ```.if```, it is met of cond evaluates to non-zero integer. ```.ifdef``` is met if the given identifier is defined anywhere in the code, and ```.ifndef``` if it is not.

### Else case of a conditional block

```
.else
.elseif cond
.elseifdef identifier
.elseifndef identifier
```

The else block is used if the condition of the condition of the if block was not met. ```.else``` unconditionally inserts the content of the else block, while the others start a new if block and work as described before.

### Ending a conditional block

```
.endif
```

Ends the lastest open if or else block.



## 4.3  ARM Directives

### Changing instruction set

```
.arm
.thumb
```

These directives can be used to select the ARM or THUMB instruction set. ```.arm``` tells the assembler to use the full 32 bit ARM instruction set, while ```.thumb``` uses the cut-down 16 bit THUMB instruction set.

### Pools

```
.pool
```

This directive works together with the pseudo opcode ```ldr rx,=value```. The immediate is added to the nearest pool, and the instruction is turned into a PC relative load. The range is limited, so you may have to define several pools.  
Example:

```
  ldr  r0,=0xFFEEDDCC
  ; ...
  .pool
```

```.pool``` will automatically align the position to a multiple of 4.

### Debug messages

```
.msg
```

Inserts a no$gba debug message as described by GBATEK.

