#pragma once

class TempData;

class CAssemblerCommand
{
public:
	CAssemblerCommand();
	virtual bool Validate() = 0;
	virtual void Encode() = 0;
	virtual void writeTempData(TempData& tempData) = 0;
	virtual bool IsConditional() { return false; };
	void SetFileInfo();
protected:
	int FileNum;
	int FileLine;
};


