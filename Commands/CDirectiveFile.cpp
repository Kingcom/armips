#include "stdafx.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/FileClasses.h"

typedef void (CDirectiveFile::*fileinitfunc)(ArgumentList&);
typedef bool (CDirectiveFile::*filevalidatefunc)();
typedef void (CDirectiveFile::*fileencodefunc)();
typedef std::wstring (CDirectiveFile::*filewritetempfunc)();

typedef struct {
	eDirectiveFileMode Mode;
	fileinitfunc InitFunc;
	filevalidatefunc ValidateFunc;
	fileencodefunc EncodeFunc;
	filewritetempfunc WriteTempFunc;
} tDirectiveFileTypes;

const tDirectiveFileTypes DirectiveFileTypes[] = {
	{ DIRECTIVEFILE_OPEN,		&CDirectiveFile::InitOpen,		&CDirectiveFile::ValidateOpen,		&CDirectiveFile::EncodeOpen,		&CDirectiveFile::WriteTempOpen },
	{ DIRECTIVEFILE_CREATE,		&CDirectiveFile::InitCreate,	&CDirectiveFile::ValidateCreate,	&CDirectiveFile::EncodeCreate,		&CDirectiveFile::WriteTempCreate },
	{ DIRECTIVEFILE_COPY,		&CDirectiveFile::InitCopy,		&CDirectiveFile::ValidateCopy,		&CDirectiveFile::EncodeCopy,		&CDirectiveFile::WriteTempCopy },
	{ DIRECTIVEFILE_CLOSE,		&CDirectiveFile::InitClose,		&CDirectiveFile::ValidateClose,		&CDirectiveFile::EncodeClose,		&CDirectiveFile::WriteTempClose },
	{ DIRECTIVEFILE_ORG,		&CDirectiveFile::InitOrg,		&CDirectiveFile::ValidateOrg,		&CDirectiveFile::EncodeOrg,			&CDirectiveFile::WriteTempOrg },
	{ DIRECTIVEFILE_ORGA,		&CDirectiveFile::InitOrga,		&CDirectiveFile::ValidateOrga,		&CDirectiveFile::EncodeOrga,		&CDirectiveFile::WriteTempOrga },
	{ DIRECTIVEFILE_INCBIN,		&CDirectiveFile::InitIncbin,	&CDirectiveFile::ValidateIncbin,	&CDirectiveFile::EncodeIncbin,		&CDirectiveFile::WriteTempIncbin },
	{ DIRECTIVEFILE_ALIGN,		&CDirectiveFile::InitAlign,		&CDirectiveFile::ValidateAlign,		&CDirectiveFile::EncodeAlign,		&CDirectiveFile::WriteTempAlign },
	{ DIRECTIVEFILE_HEADERSIZE,	&CDirectiveFile::InitHeaderSize,&CDirectiveFile::ValidateHeaderSize,&CDirectiveFile::EncodeHeaderSize,	&CDirectiveFile::WriteTempHeaderSize },
	{ DIRECTIVEFILE_INVALID,	NULL,							NULL,								NULL,								NULL }
};


// **************
// Directive Open
// **************

void CDirectiveFile::InitOpen(ArgumentList& Args)
{
	fileName = getFullPathName(Args[0].text);

//	strcpy(FileName,Args.GetEntry(0));
	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",fileName.c_str());
	}
	if (ConvertExpression(Args[1].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[1].text.c_str());
	}

	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.Section++;
}

bool CDirectiveFile::ValidateOpen()
{
	BinaryFile TempFile;

	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.FileOpened = true;

	if (TempFile.open(fileName,BinaryFile::ReadWrite) == false)
	{
		Logger::queueError(Logger::Error,L"Could not open file %s",fileName.c_str());
	} else {
		TempFile.close();
	}

	return false;
}

void CDirectiveFile::EncodeOpen()
{
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;

	if (Global.Output.open(fileName,BinaryFile::ReadWrite) == false)
	{
		Logger::printError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
	}
	Global.FileOpened = true;
}

std::wstring CDirectiveFile::WriteTempOpen()
{
	return formatString(L".open \"%s\",0x%08X",fileName.c_str(),RamAddress);
}


// ****************
// Directive Create
// ****************

void CDirectiveFile::InitCreate(ArgumentList& Args)
{
	fileName = getFullPathName(Args[0].text);

	if (ConvertExpression(Args[1].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[1].text.c_str());
	}

	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.Section++;
}

bool CDirectiveFile::ValidateCreate()
{
	BinaryFile TempFile;

	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.FileOpened = true;

	if (fileExists(fileName) == true)
	{
		if (TempFile.open(fileName,BinaryFile::ReadWrite) == false)
		{
			Logger::queueError(Logger::Error,L"Could not create file %s",fileName.c_str());
		} else {
			TempFile.close();
		}
	} else {
		if (TempFile.open(fileName,BinaryFile::Write) == false)
		{
			Logger::queueError(Logger::Error,L"Could not create file %s",fileName.c_str());
		} else {
			TempFile.close();
#ifdef USE_WINDOWS_FUNCS
			DeleteFileW(fileName.c_str());
#else
			// gute frage...
#endif
		}
	}
	return false;
}

void CDirectiveFile::EncodeCreate()
{
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;

	if (Global.Output.open(fileName,BinaryFile::Write) == false)
	{
		Logger::printError(Logger::FatalError,L"Could not create file %s",fileName.c_str());
	}
	Global.FileOpened = true;
}

std::wstring CDirectiveFile::WriteTempCreate()
{
	return formatString(L".create \"%s\",0x%08X",fileName.c_str(),RamAddress);
}


// **************
// Directive Copy
// **************

void CDirectiveFile::InitCopy(ArgumentList& Args)
{
	fileName = getFullPathName(Args[1].text);
	originalName = getFullPathName(Args[0].text);

	if (fileExists(originalName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",originalName.c_str());
	}
	if (ConvertExpression(Args[2].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[2].text.c_str());
	}
	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.Section++;
}

bool CDirectiveFile::ValidateCopy()
{
	BinaryFile TempFile;

	Global.RamPos = RamAddress;
	Global.HeaderSize = RamAddress;
	Global.FileOpened = true;

	if (TempFile.open(originalName,BinaryFile::ReadWrite) == false)
	{
		Logger::queueError(Logger::Error,L"Could not open file %s",fileName.c_str());
	} else {
		TempFile.close();
	}
	return false;
}

void CDirectiveFile::EncodeCopy()
{
#ifdef USE_WINDOWS_FUNCS
	CopyFileW(originalName.c_str(),fileName.c_str(),false);
#else
	// gute frage...
#endif
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;

	if (Global.Output.open(fileName,BinaryFile::ReadWrite) == false)
	{
		Logger::printError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
	}
	Global.FileOpened = true;
}

std::wstring CDirectiveFile::WriteTempCopy()
{
	return formatString(L".open \"%s\",\"%s\",0x%08X",originalName.c_str(),fileName.c_str(),RamAddress);
}


// ***************
// Directive Close
// ***************

void CDirectiveFile::InitClose(ArgumentList& Args)
{
	return;
}

bool CDirectiveFile::ValidateClose()
{
	Global.FileOpened = false;
	return false;
}

void CDirectiveFile::EncodeClose()
{
	if (Global.FileOpened == false)
	{
		Logger::printError(Logger::Error,L"No file opened");
		return;
	}
	Global.Output.close();
	Global.FileOpened = false;
}

std::wstring CDirectiveFile::WriteTempClose()
{
	return L".close";
}


// *************
// Directive Org
// *************

void CDirectiveFile::InitOrg(ArgumentList& Args)
{
	if (ConvertExpression(Args[0].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[0].text.c_str());
	}
	Global.RamPos = RamAddress;
	Global.Section++;
}

bool CDirectiveFile::ValidateOrg()
{	
	Global.RamPos = RamAddress;
	return false;
}

void CDirectiveFile::EncodeOrg()
{
	if (Global.FileOpened == false)
	{
		Logger::printError(Logger::Error,L"No file opened");
		return;
	}
	Global.RamPos = RamAddress;
	Global.Output.setPos(RamAddress-Global.HeaderSize);
}

std::wstring CDirectiveFile::WriteTempOrg()
{
	return formatString(L".org 0x%08X",RamAddress);
}


// **************
// Directive Orga
// **************

void CDirectiveFile::InitOrga(ArgumentList& Args)
{
	if (ConvertExpression(Args[0].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid file address %s",Args[0].text.c_str());
	}

	Global.RamPos = RamAddress+Global.HeaderSize;
	Global.Section++;
}

bool CDirectiveFile::ValidateOrga()
{
	Global.RamPos = RamAddress+Global.HeaderSize;
	return false;
}

void CDirectiveFile::EncodeOrga()
{	
	if (Global.FileOpened == false)
	{
		Logger::printError(Logger::Error,L"No file opened");
		return;
	}
	Global.RamPos = RamAddress;
	Global.Output.setPos(RamAddress);
}

std::wstring CDirectiveFile::WriteTempOrga()
{
	return formatString(L".orga 0x%08X",RamAddress);
}


// ****************
// Directive Incbin
// ****************


void CDirectiveFile::InitIncbin(ArgumentList& Args)
{
	fileName = getFullPathName(Args[0].text);

	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",fileName.c_str());
	}

	InputFileSize = fileSize(fileName);
	Global.RamPos += InputFileSize;
}

bool CDirectiveFile::ValidateIncbin()
{
	Global.RamPos += InputFileSize;
	return false;
}

void CDirectiveFile::EncodeIncbin()
{
	if (InputFileSize != 0)
	{
		ByteArray data = ByteArray::fromFile(fileName);
		if (data.size() == 0)
		{
			Logger::printError(Logger::Error,L"Could not read file \"%s\"",fileName.c_str());
			return;
		}
		Global.Output.write(data.data(),data.size());
		Global.RamPos += InputFileSize;
	}
}

std::wstring CDirectiveFile::WriteTempIncbin()
{
	return formatString(L".incbin \"%s\"",fileName.c_str());
}


// ***************
// Directive Align
// ***************

void CDirectiveFile::InitAlign(ArgumentList& Args)
{
	if (Args.size() == 1)
	{
		if (ConvertExpression(Args[0].text,Alignment) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid alignment %s",Args[0].text.c_str());
		}
		if (isPowerOfTwo(Alignment) == false)
		{
			Logger::printError(Logger::Error,L"Invalid alignment %d",Alignment);
		}
	} else {
		Alignment = Arch->GetWordSize();
	}

	int num = (((unsigned)Global.RamPos % Alignment) ? (Alignment-((unsigned)Global.RamPos%Alignment)) : 0);
	Global.RamPos += num;
}

bool CDirectiveFile::ValidateAlign()
{
	int num = ((unsigned)Global.RamPos % Alignment ? Alignment-((unsigned)Global.RamPos%Alignment) : 0);
	Global.RamPos += num;
	return false;
}

void CDirectiveFile::EncodeAlign()
{
	unsigned char AlignBuffer[128];

	int n = ((unsigned)RamPos % Alignment ? Alignment-((unsigned)RamPos%Alignment) : 0);
	Global.RamPos += n;
	memset(AlignBuffer,0,n > 128 ? 128 : n);
	while (n > 128)
	{
		Global.Output.write(AlignBuffer,128);
		n -= 128;
	}
	Global.Output.write(AlignBuffer,n);
}

std::wstring CDirectiveFile::WriteTempAlign()
{
	return formatString(L".align 0x%08X",Alignment);
}


// ********************
// Directive HeaderSize
// ********************

void CDirectiveFile::InitHeaderSize(ArgumentList& Args)
{
	if (ConvertExpression(Args[0].text,RamAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid header size %s",Args[0].text.c_str());
	}

	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;
}

bool CDirectiveFile::ValidateHeaderSize()
{
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;
	return false;
}

void CDirectiveFile::EncodeHeaderSize()
{
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;
}

std::wstring CDirectiveFile::WriteTempHeaderSize()
{
	return formatString(L".headersize 0x%08X",RamAddress);
}


// **************
// Base Functions
// **************

CDirectiveFile::CDirectiveFile(eDirectiveFileMode FileMode, ArgumentList& Args)
{
	Mode = FileMode;
	if (DirectiveFileTypes[Mode].Mode != Mode)
	{
		Logger::printError(Logger::Error,L"Internal data mismatch");
		return;
	}
	(*this.*DirectiveFileTypes[Mode].InitFunc)(Args);
}

bool CDirectiveFile::Validate()
{
	bool Result = false;
	RamPos = Global.RamPos;
	
	Result = (*this.*DirectiveFileTypes[(int)Mode].ValidateFunc)();
	Arch->NextSection();
	return Result;
}

void CDirectiveFile::Encode()
{
	(*this.*DirectiveFileTypes[(int)Mode].EncodeFunc)();
}

void CDirectiveFile::writeTempData(TempData& tempData)
{
	std::wstring text = (*this.*DirectiveFileTypes[(int)Mode].WriteTempFunc)();
	tempData.writeLine(RamPos,text);
}

void CDirectiveFile::writeSymData(SymbolData& symData)
{
	switch (Mode)
	{
	case DIRECTIVEFILE_INCBIN:
		symData.addSymbol(RamPos,formatString(L".byt:%04X",InputFileSize));
		break;
	}
}
