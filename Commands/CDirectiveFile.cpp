#include "stdafx.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/FileClasses.h"
#include "Core/FileManager.h"

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
	std::wstring fileName = getFullPathName(Args[0].text);
	int virtualAddress;

	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",fileName.c_str());
		return;
	}
	if (ConvertExpression(Args[1].text,virtualAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[1].text.c_str());
		return;
	}

	file = new GenericAssemblerFile(fileName,virtualAddress,false);
	g_fileManager->addFile(file);
	Global.Section++;
}

bool CDirectiveFile::ValidateOpen()
{
	g_fileManager->openFile(file,true);
	return false;
}

void CDirectiveFile::EncodeOpen()
{
	g_fileManager->openFile(file,false);
}

std::wstring CDirectiveFile::WriteTempOpen()
{
	return formatString(L".open \"%s\",0x%08X",file->getFileName().c_str(),file->getOriginalHeaderSize());
}


// ****************
// Directive Create
// ****************

void CDirectiveFile::InitCreate(ArgumentList& Args)
{
	std::wstring fileName = getFullPathName(Args[0].text);
	int virtualAddress;

	if (ConvertExpression(Args[1].text,virtualAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[1].text.c_str());
		return;
	}

	file = new GenericAssemblerFile(fileName,virtualAddress,true);
	g_fileManager->addFile(file);
	Global.Section++;
}

bool CDirectiveFile::ValidateCreate()
{
	g_fileManager->openFile(file,true);
	return false;
}

void CDirectiveFile::EncodeCreate()
{
	g_fileManager->openFile(file,false);
}

std::wstring CDirectiveFile::WriteTempCreate()
{
	return formatString(L".create \"%s\",0x%08X",file->getFileName().c_str(),file->getOriginalHeaderSize());
}


// **************
// Directive Copy
// **************

void CDirectiveFile::InitCopy(ArgumentList& Args)
{
	std::wstring originalName = getFullPathName(Args[0].text);
	std::wstring fileName = getFullPathName(Args[1].text);
	int virtualAddress;

	if (fileExists(originalName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",fileName.c_str());
		return;
	}
	if (ConvertExpression(Args[2].text,virtualAddress) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[1].text.c_str());
		return;
	}

	file = new GenericAssemblerFile(fileName,originalName,virtualAddress);
	g_fileManager->addFile(file);
	Global.Section++;
}

bool CDirectiveFile::ValidateCopy()
{
	g_fileManager->openFile(file,true);
	return false;
}

void CDirectiveFile::EncodeCopy()
{
	g_fileManager->openFile(file,false);
}

std::wstring CDirectiveFile::WriteTempCopy()
{
	return formatString(L".open \"%s\",\"%s\",0x%08X",file->getOriginalFileName().c_str(),
		file->getFileName().c_str(),file->getOriginalHeaderSize());
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
	g_fileManager->closeFile();
	return false;
}

void CDirectiveFile::EncodeClose()
{
	g_fileManager->closeFile();
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
	if (ConvertExpression(Args[0].text,position) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[0].text.c_str());
	}
	g_fileManager->seekVirtual(position);
	Global.Section++;
}

bool CDirectiveFile::ValidateOrg()
{	
	g_fileManager->seekVirtual(position);
	return false;
}

void CDirectiveFile::EncodeOrg()
{
	g_fileManager->seekVirtual(position);
}

std::wstring CDirectiveFile::WriteTempOrg()
{
	return formatString(L".org 0x%08X",position);
}


// **************
// Directive Orga
// **************

void CDirectiveFile::InitOrga(ArgumentList& Args)
{
	if (ConvertExpression(Args[0].text,position) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid file address %s",Args[0].text.c_str());
	}
	g_fileManager->seekPhysical(position);
	Global.Section++;
}

bool CDirectiveFile::ValidateOrga()
{
	g_fileManager->seekPhysical(position);
	return false;
}

void CDirectiveFile::EncodeOrga()
{	
	g_fileManager->seekPhysical(position);
}

std::wstring CDirectiveFile::WriteTempOrga()
{
	return formatString(L".orga 0x%08X",position);
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

	inputFileSize = fileSize(fileName);
	g_fileManager->advanceMemory(inputFileSize);
}

bool CDirectiveFile::ValidateIncbin()
{
	g_fileManager->advanceMemory(inputFileSize);
	return false;
}

void CDirectiveFile::EncodeIncbin()
{
	if (inputFileSize != 0)
	{
		ByteArray data = ByteArray::fromFile(fileName);
		if (data.size() == 0)
		{
			Logger::printError(Logger::Error,L"Could not read file \"%s\"",fileName.c_str());
			return;
		}
		g_fileManager->write(data.data(),data.size());
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
		if (ConvertExpression(Args[0].text,alignment) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid alignment %s",Args[0].text.c_str());
		}
		if (isPowerOfTwo(alignment) == false)
		{
			Logger::printError(Logger::Error,L"Invalid alignment %d",alignment);
		}
	} else {
		alignment = Arch->GetWordSize();
	}

	unsigned int mod = g_fileManager->getVirtualAddress() % alignment;
	unsigned int num = mod ? alignment-mod : 0;
	g_fileManager->advanceMemory(num);
}

bool CDirectiveFile::ValidateAlign()
{
	unsigned int mod = g_fileManager->getVirtualAddress() % alignment;
	unsigned int num = mod ? alignment-mod : 0;
	g_fileManager->advanceMemory(num);
	return false;
}

void CDirectiveFile::EncodeAlign()
{
	unsigned char AlignBuffer[128];
	
	unsigned int mod = g_fileManager->getVirtualAddress() % alignment;
	unsigned int n = mod ? alignment-mod : 0;
	memset(AlignBuffer,0,n > 128 ? 128 : n);
	while (n > 128)
	{
		g_fileManager->write(AlignBuffer,128);
		n -= 128;
	}
	g_fileManager->write(AlignBuffer,n);
}

std::wstring CDirectiveFile::WriteTempAlign()
{
	return formatString(L".align 0x%08X",alignment);
}


// ********************
// Directive HeaderSize
// ********************

void CDirectiveFile::InitHeaderSize(ArgumentList& Args)
{
	if (ConvertExpression(Args[0].text,position) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid header size %s",Args[0].text.c_str());
	}

	GenericAssemblerFile* file = dynamic_cast<GenericAssemblerFile*>(g_fileManager->getOpenFile());
	if (file == NULL)
	{
		Logger::printError(Logger::Error,L"Header size not applicable for this file");
		return;
	}
	file->setHeaderSize(position);
}

bool CDirectiveFile::ValidateHeaderSize()
{
	GenericAssemblerFile* file = dynamic_cast<GenericAssemblerFile*>(g_fileManager->getOpenFile());
	if (file == NULL)
	{
		Logger::queueError(Logger::Error,L"Header size not applicable for this file");
		return false;
	}
	file->setHeaderSize(position);
	return false;
}

void CDirectiveFile::EncodeHeaderSize()
{
	GenericAssemblerFile* file = dynamic_cast<GenericAssemblerFile*>(g_fileManager->getOpenFile());
	if (file == NULL)
	{
		Logger::printError(Logger::Error,L"Header size not applicable for this file");
		return;
	}
	file->setHeaderSize(position);
}

std::wstring CDirectiveFile::WriteTempHeaderSize()
{
	return formatString(L".headersize 0x%08X",position);
}


// **************
// Base Functions
// **************

CDirectiveFile::CDirectiveFile(eDirectiveFileMode FileMode, ArgumentList& Args)
{
	file = NULL;
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
	tempData.writeLine(g_fileManager->getVirtualAddress(),text);
}

void CDirectiveFile::writeSymData(SymbolData& symData)
{
	switch (Mode)
	{
	case DIRECTIVEFILE_INCBIN:
		symData.addSymbol(g_fileManager->getVirtualAddress(),formatString(L".byt:%04X",inputFileSize));
		break;
	}
}
