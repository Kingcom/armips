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
	CDirectiveFile(eDirectiveFileMode FileMode, CArgumentList& Args);
	~CDirectiveFile();
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
	//  Open
	void InitOpen(CArgumentList& Args);
	bool ValidateOpen();
	void EncodeOpen();
	void WriteTempOpen(char* str);
	// Create
	void InitCreate(CArgumentList& Args);
	bool ValidateCreate();
	void EncodeCreate();
	void WriteTempCreate(char* str);
	// Copy
	void InitCopy(CArgumentList& Args);
	bool ValidateCopy();
	void EncodeCopy();
	void WriteTempCopy(char* str);
	// Close
	void InitClose(CArgumentList& Args);
	bool ValidateClose();
	void EncodeClose();
	void WriteTempClose(char* str);
	// Org
	void InitOrg(CArgumentList& Args);
	bool ValidateOrg();
	void EncodeOrg();
	void WriteTempOrg(char* str);
	// Orga
	void InitOrga(CArgumentList& Args);
	bool ValidateOrga();
	void EncodeOrga();
	void WriteTempOrga(char* str);
	// Incbin
	void InitIncbin(CArgumentList& Args);
	bool ValidateIncbin();
	void EncodeIncbin();
	void WriteTempIncbin(char* str);
	// Align
	void InitAlign(CArgumentList& Args);
	bool ValidateAlign();
	void EncodeAlign();
	void WriteTempAlign(char* str);
	// HeaderSize
	void InitHeaderSize(CArgumentList& Args);
	bool ValidateHeaderSize();
	void EncodeHeaderSize();
	void WriteTempHeaderSize(char* str);
private:
	eDirectiveFileMode Mode;
	char FileName[255];
	char OriginalName[255];
	int RamAddress;
	int InputFileSize;
	int Alignment;
	int RamPos;
	CExpressionCommandList FillExpression;
	int FillByte;
};
