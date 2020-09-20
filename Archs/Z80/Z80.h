#pragma once

#include "Archs/Architecture.h"
#include "Core/ELF/ElfRelocator.h"
#include "Core/Expression.h"

struct Z80RegisterValue
{
	std::wstring name;
	int num;
};

class CZ80Architecture: public CArchitecture
{
public:
	CZ80Architecture();

	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser);
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual std::unique_ptr<IElfRelocator> getElfRelocator() { return 0; };
	virtual Endianness getEndianness() { return Endianness::Little; };
};

extern CZ80Architecture Z80;
