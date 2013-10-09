#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "Core/MathParser.h"
typedef enum eDirectiveFileMode {
	DIRECTIVEFILE_OPEN,
	DIRECTIVEFILE_CREATE,
	DIRECTIVEFILE_COPY,
	DIRECTIVEFILE_CLOSE,
	DIRECTIVEFILE_ORG,
	DIRECTIVEFILE_ORGA,
	DIRECTIVEFILE_INCBIN,
	DIRECTIVEFILE_ALIGN,
	DIRECTIVEFILE_HEADERSIZE,
	DIRECTIVEFILE_INVALID
};


class CDirectiveFile: public CAssemblerCommand
{
public:
	CDirectiveFile(eDirectiveFileMode FileMode, ArgumentList& Args);
	~CDirectiveFile();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	//  Open
	void InitOpen(ArgumentList& Args);
	bool ValidateOpen();
	void EncodeOpen();
	std::wstring WriteTempOpen();
	// Create
	void InitCreate(ArgumentList& Args);
	bool ValidateCreate();
	void EncodeCreate();
	std::wstring WriteTempCreate();
	// Copy
	void InitCopy(ArgumentList& Args);
	bool ValidateCopy();
	void EncodeCopy();
	std::wstring WriteTempCopy();
	// Close
	void InitClose(ArgumentList& Args);
	bool ValidateClose();
	void EncodeClose();
	std::wstring WriteTempClose();
	// Org
	void InitOrg(ArgumentList& Args);
	bool ValidateOrg();
	void EncodeOrg();
	std::wstring WriteTempOrg();
	// Orga
	void InitOrga(ArgumentList& Args);
	bool ValidateOrga();
	void EncodeOrga();
	std::wstring WriteTempOrga();
	// Incbin
	void InitIncbin(ArgumentList& Args);
	bool ValidateIncbin();
	void EncodeIncbin();
	std::wstring WriteTempIncbin();
	// Align
	void InitAlign(ArgumentList& Args);
	bool ValidateAlign();
	void EncodeAlign();
	std::wstring WriteTempAlign();
	// HeaderSize
	void InitHeaderSize(ArgumentList& Args);
	bool ValidateHeaderSize();
	void EncodeHeaderSize();
	std::wstring WriteTempHeaderSize();
private:
	eDirectiveFileMode Mode;
	std::wstring fileName;
	std::wstring originalName;
	int RamAddress;
	int InputFileSize;
	int Alignment;
	int RamPos;
	CExpressionCommandList FillExpression;
	int FillByte;
};
