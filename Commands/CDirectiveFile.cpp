#include "stdafx.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/FileClasses.h"

// TODO: GUCKEN OB ALLES AUCH FUNKTIONIERT

typedef void (CDirectiveFile::*fileinitfunc)(CArgumentList&);
typedef bool (CDirectiveFile::*filevalidatefunc)();
typedef void (CDirectiveFile::*fileencodefunc)();
typedef void (CDirectiveFile::*filewritetempfunc)(char*);

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

void CDirectiveFile::InitOpen(CArgumentList& Args)
{
	getFullPathName(FileName,Args.GetEntry(0));

//	strcpy(FileName,Args.GetEntry(0));
	if (FileExists(FileName) == false)
	{
		PrintError(ERROR_FATALERROR,"File %s not found",FileName);
	}
	if (ConvertExpression(Args.GetEntry(1),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid ram address %s",Args.GetEntry(1));
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

	if (TempFile.open(FileName,BinaryFile::ReadWrite) == false)
	{
		QueueError(ERROR_ERROR,"Could not open file %s",FileName);
	} else {
		TempFile.close();
	}

	return false;
}

void CDirectiveFile::EncodeOpen()
{
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;

	if (Global.Output.open(FileName,BinaryFile::ReadWrite) == false)
	{
		PrintError(ERROR_FATALERROR,"Could not open file %s",FileName);
	}
	Global.FileOpened = true;
}

void CDirectiveFile::WriteTempOpen(char* str)
{
	sprintf(str,".open \"%s\",0x%08X",FileName,RamAddress);
}


// ****************
// Directive Create
// ****************

void CDirectiveFile::InitCreate(CArgumentList& Args)
{
	getFullPathName(FileName,Args.GetEntry(0));

//	strcpy(FileName,Args.GetEntry(0));
	if (ConvertExpression(Args.GetEntry(1),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid ram address %s",Args.GetEntry(1));
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

	if (FileExists(FileName) == true)
	{
		if (TempFile.open(FileName,BinaryFile::ReadWrite) == false)
		{
			QueueError(ERROR_ERROR,"Could not create file %s",FileName);
		} else {
			TempFile.close();
		}
	} else {
		if (TempFile.open(FileName,BinaryFile::Write) == false)
		{
			QueueError(ERROR_ERROR,"Could not create file %s",FileName);
		} else {
			TempFile.close();
#ifdef USE_WINDOWS_FUNCS
			DeleteFile(FileName);
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

	if (Global.Output.open(FileName,BinaryFile::Write) == false)
	{
		PrintError(ERROR_FATALERROR,"Could not create file %s",FileName);
	}
	Global.FileOpened = true;
}

void CDirectiveFile::WriteTempCreate(char* str)
{
	sprintf(str,".create \"%s\",0x%08X",FileName,RamAddress);
}


// **************
// Directive Copy
// **************

void CDirectiveFile::InitCopy(CArgumentList& Args)
{
	getFullPathName(FileName,Args.GetEntry(1));
	getFullPathName(OriginalName,Args.GetEntry(0));

	if (FileExists(OriginalName) == false)
	{
		PrintError(ERROR_FATALERROR,"File %s not found",OriginalName);
	}
	if (ConvertExpression(Args.GetEntry(2),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid ram address %s",Args.GetEntry(2));
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

	if (TempFile.open(OriginalName,BinaryFile::ReadWrite) == false)
	{
		QueueError(ERROR_ERROR,"Could not open file %s",FileName);
	} else {
		TempFile.close();
	}
	return false;
}

void CDirectiveFile::EncodeCopy()
{
#ifdef USE_WINDOWS_FUNCS
	CopyFile(OriginalName,FileName,false);
#else
	// gute frage...
#endif
	Global.HeaderSize = RamAddress;
	Global.RamPos = RamAddress;

	if (Global.Output.open(FileName,BinaryFile::ReadWrite) == false)
	{
		PrintError(ERROR_FATALERROR,"Could not open file %s",FileName);
	}
	Global.FileOpened = true;
}

void CDirectiveFile::WriteTempCopy(char* str)
{
	sprintf(str,".open \"%s\",\"%s\",0x%08X",OriginalName,FileName,RamAddress);
}


// ***************
// Directive Close
// ***************

void CDirectiveFile::InitClose(CArgumentList& Args)
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
		PrintError(ERROR_ERROR,"No file opened");
		return;
	}
	Global.Output.close();
	Global.FileOpened = false;
}

void CDirectiveFile::WriteTempClose(char* str)
{
	sprintf(str,".close");
}


// *************
// Directive Org
// *************

void CDirectiveFile::InitOrg(CArgumentList& Args)
{
	if (ConvertExpression(Args.GetEntry(0),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid ram address %s",Args.GetEntry(0));
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
		PrintError(ERROR_ERROR,"No file opened");
		return;
	}
	Global.RamPos = RamAddress;
	Global.Output.setPos(RamAddress-Global.HeaderSize);
}

void CDirectiveFile::WriteTempOrg(char* str)
{
	sprintf(str,".org 0x%08X",RamAddress);
}


// **************
// Directive Orga
// **************

void CDirectiveFile::InitOrga(CArgumentList& Args)
{
	if (ConvertExpression(Args.GetEntry(0),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid file address %s",Args.GetEntry(0));
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
		PrintError(ERROR_ERROR,"No file opened");
		return;
	}
	Global.RamPos = RamAddress;
	Global.Output.setPos(RamAddress);
}

void CDirectiveFile::WriteTempOrga(char* str)
{
	sprintf(str,".orga 0x%08X",RamAddress);
}


// ****************
// Directive Incbin
// ****************


void CDirectiveFile::InitIncbin(CArgumentList& Args)
{
	getFullPathName(FileName,Args.GetEntry(0));

	if (FileExists(FileName) == false)
	{
		PrintError(ERROR_FATALERROR,"File %s not found",FileName);
	}

	InputFileSize = FileSize(FileName);
	Global.RamPos += InputFileSize;
}

bool CDirectiveFile::ValidateIncbin()
{
	Global.RamPos += InputFileSize;
	return false;
}

void CDirectiveFile::EncodeIncbin()
{
	int n;
	unsigned char* Buffer = NULL;

	if ((n = ReadFileToBuffer(FileName,&Buffer)) == -1)
	{
		PrintError(ERROR_ERROR,"Could not read file \"%s\"",FileName);
		return;
	}
	Global.Output.write(Buffer,InputFileSize);
	Global.RamPos += InputFileSize;
	free(Buffer);

	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X .byt:%04X\n",RamPos,InputFileSize);
	}
}

void CDirectiveFile::WriteTempIncbin(char* str)
{
	sprintf(str,".incbin \"%s\"",FileName);
}


// ***************
// Directive Align
// ***************

void CDirectiveFile::InitAlign(CArgumentList& Args)
{
	if (Args.GetCount() == 1)
	{
		if (ConvertExpression(Args.GetEntry(0),Alignment) == false)
		{
			PrintError(ERROR_FATALERROR,"Invalid alignment %s",Args.GetEntry(0));
		}
		if (isPowerOfTwo(Alignment) == false)
		{
			PrintError(ERROR_ERROR,"Invalid alignment %d",Alignment);
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

void CDirectiveFile::WriteTempAlign(char* str)
{
	sprintf(str,".align 0x%08X",Alignment);
}


// ********************
// Directive HeaderSize
// ********************

void CDirectiveFile::InitHeaderSize(CArgumentList& Args)
{
	if (ConvertExpression(Args.GetEntry(0),RamAddress) == false)
	{
		PrintError(ERROR_FATALERROR,"Invalid header size %s",Args.GetEntry(0));
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

void CDirectiveFile::WriteTempHeaderSize(char* str)
{
	sprintf(str,".headersize 0x%08X",RamAddress);
}


// **************
// Base Functions
// **************

CDirectiveFile::CDirectiveFile(eDirectiveFileMode FileMode, CArgumentList& Args)
{
	Mode = FileMode;
	if (DirectiveFileTypes[Mode].Mode != Mode)
	{
		PrintError(ERROR_ERROR,"Internal data mismatch");
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

void CDirectiveFile::WriteTempData(FILE*& Output)
{
	char str[512];

	(*this.*DirectiveFileTypes[(int)Mode].WriteTempFunc)(str);
	WriteToTempData(Output,str,RamPos);
}
