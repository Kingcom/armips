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
	CDirectiveData(CArgumentList& Args, int SizePerUnit, bool asc);
	~CDirectiveData();
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
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

