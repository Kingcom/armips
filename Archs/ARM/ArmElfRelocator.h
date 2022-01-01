#pragma once

#include "Core/ELF/ElfRelocator.h"

enum {
	R_ARM_ABS32 = 2,
	R_ARM_THM_CALL = 10,
	R_ARM_CALL = 28,
	R_ARM_JUMP24 = 29,
	R_ARM_TARGET1 = 38,
	R_ARM_V4BX = 40,
};


class ArmElfRelocator: public IElfRelocator
{
public:
	ArmElfRelocator(bool arm9): arm9(arm9) { }
	int expectedMachine() const override;
	bool isDummyRelocationType(int type) const override;

	bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::string>& errors) override;
	void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) override;

	std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors) override;
private:
	bool arm9;
};
