#pragma once
#include "Core/ELF/ElfRelocator.h"

enum {
	R_MIPS_NONE,
	R_MIPS_16,
	R_MIPS_32,
	R_MIPS_REL32,
	R_MIPS_26,
	R_MIPS_HI16,
	R_MIPS_LO16,
	R_MIPS_GPREL16,
	R_MIPS_LITERAL,
	R_MIPS_GOT16,
	R_MIPS_PC16,
	R_MIPS_CALL16,
	R_MIPS_GPREL32
};

class MipsElfRelocator: public IElfRelocator
{
public:
	int expectedMachine() const override;
	bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::string>& errors) override;
	bool finish(std::vector<RelocationAction>& actions, std::vector<std::string>& errors) override;
	void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) override;
	std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors) override;
private:
	bool processHi16Entries(uint32_t lo16Opcode, int64_t lo16RelocationBase, std::vector<RelocationAction>& actions, std::vector<std::string>& errors);

	struct Hi16Entry
	{
		Hi16Entry(int64_t offset, int64_t relocationBase, uint32_t opcode) : offset(offset), relocationBase(relocationBase), opcode(opcode) {}
		int64_t offset;
		int64_t relocationBase;
		uint32_t opcode;
	};

	std::vector<Hi16Entry> hi16Entries;
};
