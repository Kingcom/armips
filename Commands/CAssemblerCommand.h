#pragma once

class TempData;
class SymbolData;

class CAssemblerCommand
{
public:
	CAssemblerCommand();
	virtual ~CAssemblerCommand() { };
	virtual bool Validate() = 0;
	virtual void Encode() = 0;
	virtual void writeTempData(TempData& tempData) = 0;
	virtual void writeSymData(SymbolData& symData) { };
	virtual bool IsConditional() { return false; };
	virtual bool IsPool() { return false; };
	void SetFileInfo();
	int getSection() { return section; }
	void updateSection(int num) { section = num; }
protected:
	int FileNum;
	int FileLine;
private:
	int section;
};


