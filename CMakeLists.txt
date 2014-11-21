cmake_minimum_required(VERSION 2.8)
project(armips) 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)

add_executable(armips
	Main/main.cpp
	Main/Tests.cpp
	Main/Tests.h
	stdafx.cpp
	stdafx.h

	Util/EncodingTable.cpp
	Util/EncodingTable.h
	Util/Util.cpp
	Util/Util.h
	Util/CommonClasses.cpp
	Util/CommonClasses.h
	Util/CRC.cpp
	Util/CRC.h
	Util/FileClasses.cpp
	Util/FileClasses.h
	Util/ByteArray.cpp
	Util/ByteArray.h
	Util/StringFormat.cpp
	Util/StringFormat.h
	
	Core/ELF/ElfTypes.h
	Core/ELF/ElfRelocator.cpp
	Core/ELF/ElfRelocator.h
	Core/ELF/ElfFile.cpp
	Core/ELF/ElfFile.h
	Core/FileManager.cpp
	Core/FileManager.h
	Core/SymbolTable.cpp
	Core/SymbolTable.h
	Core/SymbolData.cpp
	Core/SymbolData.h
	Core/Common.cpp
	Core/Common.h
	Core/Misc.cpp
	Core/Misc.h
	Core/MathParser.cpp
	Core/MathParser.h
	Core/Directives.cpp
	Core/Directives.h
	Core/Assembler.cpp
	Core/Assembler.h
	Core/CMacro.cpp
	Core/CMacro.h
	
	Commands/CDirectiveFill.cpp
	Commands/CDirectiveFill.h
	Commands/CDirectiveFile.cpp
	Commands/CDirectiveFile.h
	Commands/CDirectiveData.cpp
	Commands/CDirectiveData.h
	Commands/CAssemblerCommand.cpp
	Commands/CAssemblerCommand.h
	Commands/CDirectiveArea.cpp
	Commands/CDirectiveArea.h
	Commands/CAssemblerLabel.cpp
	Commands/CAssemblerLabel.h
	Commands/CDirectiveMessage.cpp
	Commands/CDirectiveMessage.h
	Commands/CDirectiveConditional.cpp
	Commands/CDirectiveConditional.h

	Archs/Architecture.h
	Archs/Architecture.cpp

	Archs/ARM/Arm.cpp
	Archs/ARM/Arm.h
	Archs/ARM/ArmOpcodes.cpp
	Archs/ARM/ArmOpcodes.h
	Archs/ARM/ArmRelocator.cpp
	Archs/ARM/ArmRelocator.h
	Archs/ARM/Pool.cpp
	Archs/ARM/Pool.h
	Archs/ARM/ThumbOpcodes.cpp
	Archs/ARM/ThumbOpcodes.h
	Archs/ARM/CThumbInstruction.cpp
	Archs/ARM/CArmInstruction.h
	Archs/ARM/CArmInstruction.cpp
	Archs/ARM/CThumbInstruction.h

	Archs/MIPS/Mips.cpp
	Archs/MIPS/Mips.h
	Archs/MIPS/MipsPSP.cpp
	Archs/MIPS/MipsPSP.h
	Archs/MIPS/PsxRelocator.cpp
	Archs/MIPS/PsxRelocator.h
	Archs/MIPS/MipsElfFile.cpp
	Archs/MIPS/MipsElfFile.h
	Archs/MIPS/MipsOpcodes.cpp
	Archs/MIPS/MipsOpcodes.h
	Archs/MIPS/CMipsInstruction.cpp
	Archs/MIPS/CMipsInstruction.h
	Archs/MIPS/MipsMacros.cpp
	Archs/MIPS/MipsMacros.h
	Archs/MIPS/CMipsMacro.cpp
	Archs/MIPS/CMipsMacro.h

	Archs/Z80/CZ80Instruction.cpp
	Archs/Z80/CZ80Instruction.h
	Archs/Z80/z80Opcodes.cpp
	Archs/Z80/z80Opcodes.h
	Archs/Z80/z80.cpp
	Archs/Z80/z80.h
)
