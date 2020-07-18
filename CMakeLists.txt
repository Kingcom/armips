cmake_minimum_required(VERSION 3.13)
project(armips) 

option(ARMIPS_REGEXP "Enable regexp expression functions" ON)

# Helper function to set common parameters for a target
function(init_target TARGET_NAME)
	set_target_properties(${TARGET_NAME} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED TRUE
		CXX_EXTENSIONS FALSE)

	target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

	if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
		target_compile_options(${TARGET_NAME} PRIVATE -Wall)
		target_compile_options(${TARGET_NAME} PRIVATE -Wno-uninitialized)
		target_compile_options(${TARGET_NAME} PRIVATE -Wno-unused-parameter)

		if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Haiku" AND NOT WIIU)
			find_package(Threads REQUIRED)
			target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
		endif()
	endif()

	if (ARMIPS_REGEXP)
		target_compile_definitions(${TARGET_NAME} PRIVATE ARMIPS_REGEXP=1)
	endif()

	if (WIN32)
		if (MSVC)
			target_compile_definitions(${TARGET_NAME} PRIVATE UNICODE _UNICODE)
			target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
		else()
			target_compile_options(${TARGET_NAME} PRIVATE -municode)
			target_link_options(${TARGET_NAME} PRIVATE -municode)
		endif()
	endif()
endfunction()

# Helper function to automatically set source groups for all source files
function(armips_target_sources TARGET SCOPE)
	target_sources("${TARGET}" "${SCOPE}" ${ARGN})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGN})
endfunction()

# tinyformat
add_library(tinyformat INTERFACE)
target_include_directories(tinyformat INTERFACE ext/tinyformat)

# libarmips
add_library(armips STATIC)
init_target(armips)
target_link_libraries(armips PUBLIC tinyformat)

if (WIN32)
	target_link_libraries(armips PUBLIC shlwapi)
endif()

armips_target_sources(armips PRIVATE
	Archs/Architecture.h
	Archs/Architecture.cpp

	Archs/ARM/Arm.cpp
	Archs/ARM/Arm.h
	Archs/ARM/ArmExpressionFunctions.cpp
	Archs/ARM/ArmExpressionFunctions.h
	Archs/ARM/ArmElfRelocator.cpp
	Archs/ARM/ArmElfRelocator.h
	Archs/ARM/ArmOpcodes.cpp
	Archs/ARM/ArmOpcodes.h
	Archs/ARM/ArmParser.cpp
	Archs/ARM/ArmParser.h
	Archs/ARM/CArmInstruction.cpp
	Archs/ARM/CArmInstruction.h
	Archs/ARM/CThumbInstruction.cpp
	Archs/ARM/CThumbInstruction.h
	Archs/ARM/Pool.cpp
	Archs/ARM/Pool.h
	Archs/ARM/ThumbOpcodes.cpp
	Archs/ARM/ThumbOpcodes.h

	Archs/MIPS/CMipsInstruction.cpp
	Archs/MIPS/CMipsInstruction.h
	Archs/MIPS/Mips.cpp
	Archs/MIPS/Mips.h
	Archs/MIPS/MipsElfFile.cpp
	Archs/MIPS/MipsElfFile.h
	Archs/MIPS/MipsExpressionFunctions.cpp
	Archs/MIPS/MipsExpressionFunctions.h
	Archs/MIPS/MipsElfRelocator.cpp
	Archs/MIPS/MipsElfRelocator.h
	Archs/MIPS/MipsMacros.cpp
	Archs/MIPS/MipsMacros.h
	Archs/MIPS/MipsOpcodes.cpp
	Archs/MIPS/MipsOpcodes.h
	Archs/MIPS/MipsParser.cpp
	Archs/MIPS/MipsParser.h
	Archs/MIPS/PsxRelocator.cpp
	Archs/MIPS/PsxRelocator.h
	
	Commands/CAssemblerCommand.cpp
	Commands/CAssemblerCommand.h
	Commands/CAssemblerLabel.cpp
	Commands/CAssemblerLabel.h
	Commands/CDirectiveArea.cpp
	Commands/CDirectiveArea.h
	Commands/CDirectiveConditional.cpp
	Commands/CDirectiveConditional.h
	Commands/CDirectiveData.cpp
	Commands/CDirectiveData.h
	Commands/CDirectiveFile.cpp
	Commands/CDirectiveFile.h
	Commands/CDirectiveMessage.cpp
	Commands/CDirectiveMessage.h
	Commands/CommandSequence.cpp
	Commands/CommandSequence.h

	Core/ELF/ElfTypes.h
	Core/ELF/ElfRelocator.cpp
	Core/ELF/ElfRelocator.h
	Core/ELF/ElfFile.cpp
	Core/ELF/ElfFile.h
	Core/Allocations.cpp
	Core/Allocations.h
	Core/Assembler.cpp
	Core/Assembler.h
	Core/Common.cpp
	Core/Common.h
	Core/Expression.cpp
	Core/Expression.h
	Core/ExpressionFunctions.cpp
	Core/ExpressionFunctions.h
	Core/FileManager.cpp
	Core/FileManager.h
	Core/Misc.cpp
	Core/Misc.h
	Core/SymbolData.cpp
	Core/SymbolData.h
	Core/SymbolTable.cpp
	Core/SymbolTable.h

	Parser/DirectivesParser.cpp
	Parser/DirectivesParser.h
	Parser/ExpressionParser.cpp
	Parser/ExpressionParser.h
	Parser/Parser.cpp
	Parser/Parser.h
	Parser/Tokenizer.cpp
	Parser/Tokenizer.h
	
	Util/ByteArray.cpp
	Util/ByteArray.h
	Util/CRC.cpp
	Util/CRC.h
	Util/EncodingTable.cpp
	Util/EncodingTable.h
	Util/FileClasses.cpp
	Util/FileClasses.h
	Util/Util.cpp
	Util/Util.h
)

# armips-bin
add_executable(armips-bin)
init_target(armips-bin)
set_target_properties(armips-bin PROPERTIES OUTPUT_NAME armips)
target_link_libraries(armips-bin PRIVATE armips)

armips_target_sources(armips-bin PRIVATE
	Main/CommandLineInterface.cpp
	Main/CommandLineInterface.h
	Main/main.cpp
	Main/Tests.cpp
	Main/Tests.h
)

# tests
add_executable(armipstests)
init_target(armipstests)
target_compile_definitions(armipstests PUBLIC ARMIPS_TESTS)
target_link_libraries(armipstests PRIVATE armips)

armips_target_sources(armipstests PRIVATE
	Main/CommandLineInterface.cpp
	Main/CommandLineInterface.h
	Main/main.cpp
	Main/Tests.cpp
	Main/Tests.h
)

enable_testing()
add_test(NAME armipstests COMMAND armipstests ${CMAKE_CURRENT_SOURCE_DIR}/Tests)

# install
install(TARGETS armips-bin RUNTIME DESTINATION .)
install(FILES Readme.md DESTINATION .)
