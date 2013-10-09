#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

typedef struct {
	bool String;
	int num;
} tDirectiveDataEntry;

class CDirectiveData: public CAssemblerCommand
{
public:
	CDirectiveData(ArgumentList& Args, int SizePerUnit, bool asc);
	~CDirectiveData();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	tDirectiveDataEntry* Entries;
	CExpressionCommandList* ExpData;
	CByteList StrData;
	int TotalAmount;
	int StrAmount;
	int ExpAmount;
	int UnitSize;
	int SpaceNeeded;
	int RamPos;
	bool ascii;
};

