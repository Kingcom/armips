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
	virtual int expectedMachine() const;
	virtual bool relocateOpcode(int type, RelocationData& data);
	virtual void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType);
	virtual std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors);
};
