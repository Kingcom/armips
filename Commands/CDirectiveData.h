#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"
#include "../Util/CommonClasses.h"

typedef struct {
	bool String;
	size_t num;
} tDirectiveDataEntry;

class CDirectiveData: public CAssemblerCommand
{
public:
	CDirectiveData(ArgumentList& Args, size_t SizePerUnit, bool asc);
	~CDirectiveData();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
private:
	std::vector<Expression> entries;
	size_t UnitSize;
	size_t SpaceNeeded;
	u64 RamPos;
	bool ascii;
};

