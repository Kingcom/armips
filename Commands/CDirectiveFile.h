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
	virtual void WriteTempData(FILE*& Output);
	//  Open
	void InitOpen(ArgumentList& Args);
	bool ValidateOpen();
	void EncodeOpen();
	void WriteTempOpen(char* str);
	// Create
	void InitCreate(ArgumentList& Args);
	bool ValidateCreate();
	void EncodeCreate();
	void WriteTempCreate(char* str);
	// Copy
	void InitCopy(ArgumentList& Args);
	bool ValidateCopy();
	void EncodeCopy();
	void WriteTempCopy(char* str);
	// Close
	void InitClose(ArgumentList& Args);
	bool ValidateClose();
	void EncodeClose();
	void WriteTempClose(char* str);
	// Org
	void InitOrg(ArgumentList& Args);
	bool ValidateOrg();
	void EncodeOrg();
	void WriteTempOrg(char* str);
	// Orga
	void InitOrga(ArgumentList& Args);
	bool ValidateOrga();
	void EncodeOrga();
	void WriteTempOrga(char* str);
	// Incbin
	void InitIncbin(ArgumentList& Args);
	bool ValidateIncbin();
	void EncodeIncbin();
	void WriteTempIncbin(char* str);
	// Align
	void InitAlign(ArgumentList& Args);
	bool ValidateAlign();
	void EncodeAlign();
	void WriteTempAlign(char* str);
	// HeaderSize
	void InitHeaderSize(ArgumentList& Args);
	bool ValidateHeaderSize();
	void EncodeHeaderSize();
	void WriteTempHeaderSize(char* str);
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
