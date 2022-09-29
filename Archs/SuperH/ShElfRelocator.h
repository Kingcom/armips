#pragma once
#include "Core/ELF/ElfRelocator.h"

enum {
	R_SH_NONE,
	R_SH_DIR32,
	R_SH_REL32,
	R_SH_DIR8WPN,
	R_SH_IND12W,
	R_SH_DIR8WPL,
	R_SH_DIR8WPZ,
	R_SH_DIR8BP,
	R_SH_DIR8W,
	R_SH_DIR8L
};

class ShElfRelocator: public IElfRelocator
{
public:
	int expectedMachine() const override;
	bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::string>& errors) override;
	bool finish(std::vector<RelocationAction>& actions, std::vector<std::string>& errors) override;
	void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) override;
	std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors) override;
private:
};
