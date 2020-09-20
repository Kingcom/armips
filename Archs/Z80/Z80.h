#pragma once

#include "Archs/Architecture.h"
#include "Core/ELF/ElfRelocator.h"
#include "Core/Expression.h"

enum class Z80ArchType { Z80 = 0, Gameboy, Invalid };

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
	void SetVersion(Z80ArchType v) { Version = v; };
	Z80ArchType GetVersion() { return Version; };
	const std::wstring GetName() {
		switch (Version)
		{
		case Z80ArchType::Gameboy:
			return L"Gameboy";
		default:
			return L"Z80";
		}
	}
private:
	Z80ArchType Version;
};

extern CZ80Architecture Z80;
