#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

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
	tDirectiveDataEntry* Entries;
	CExpressionCommandList* ExpData;
	CByteList StrData;
	size_t TotalAmount;
	size_t StrAmount;
	size_t ExpAmount;
	size_t UnitSize;
	size_t SpaceNeeded;
	u64 RamPos;
	bool ascii;
};

