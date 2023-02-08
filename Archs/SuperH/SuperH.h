#pragma once

#include "Archs/Architecture.h"
#include "Core/Types.h"

enum ShArchType { SHARCH_SATURN = 0, SHARCH_LITTLE, SHARCH_BIG, SHARCH_INVALID };

class CShArchitecture: public Architecture
{
public:
	CShArchitecture();

	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser);
	virtual void registerExpressionFunctions(ExpressionFunctionHandler &handler);
	virtual void NextSection();
	virtual void Pass2() { return; }
	virtual void Revalidate();
	virtual std::unique_ptr<IElfRelocator> getElfRelocator();
	virtual Endianness getEndianness() { return version == SHARCH_LITTLE ? Endianness::Little : Endianness::Big; };
	virtual int getWordSize() { return 2; };
	void setVersion(ShArchType type) { version = type; }
	ShArchType getVersion() { return version; }

private:
	ShArchType version;
};

extern CShArchitecture SuperH;

