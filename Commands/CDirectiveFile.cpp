#include "stdafx.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/FileClasses.h"
#include "Core/FileManager.h"


//
// CDirectiveFile
//

CDirectiveFile::CDirectiveFile(Type type, ArgumentList& args)
	: type(type)
{
	file = NULL;
	
	std::wstring originalName;
	std::wstring fileName;
	int virtualAddress;

	switch (type)
	{
	case Type::Open:
		fileName = getFullPathName(args[0].text);

		if (fileExists(fileName) == false)
		{
			Logger::printError(Logger::FatalError,L"File %s not found",fileName);
			return;
		}
		if (ConvertExpression(args[1].text,virtualAddress) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid ram address %s",args[1].text);
			return;
		}

		file = new GenericAssemblerFile(fileName,virtualAddress,false);
		break;
	case Type::Create:
		fileName = getFullPathName(args[0].text);

		if (ConvertExpression(args[1].text,virtualAddress) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid ram address %s",args[1].text);
			return;
		}

		file = new GenericAssemblerFile(fileName,virtualAddress,true);
		break;
	case Type::Copy:
		originalName = getFullPathName(args[0].text);
		fileName = getFullPathName(args[1].text);

		if (fileExists(originalName) == false)
		{
			Logger::printError(Logger::FatalError,L"File %s not found",originalName);
			return;
		}
		if (ConvertExpression(args[2].text,virtualAddress) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid ram address %s",args[2].text);
			return;
		}

		file = new GenericAssemblerFile(fileName,originalName,virtualAddress);
		break;
	case Type::Close:
		return;
	}

	g_fileManager->addFile(file);
	Global.Section++;
}


bool CDirectiveFile::Validate()
{
	Arch->NextSection();

	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		g_fileManager->openFile(file,true);
		return false;
	case Type::Close:
		g_fileManager->closeFile();
		return false;
	}

	return false;
}

void CDirectiveFile::Encode()
{
	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		g_fileManager->openFile(file,false);
		break;
	case Type::Close:
		g_fileManager->closeFile();
		break;
	}
}

void CDirectiveFile::writeTempData(TempData& tempData)
{
	std::wstring str;
	switch (type)
	{
	case Type::Open:
		str = formatString(L".open \"%s\",0x%08X",file->getFileName(),file->getOriginalHeaderSize());;
		break;
	case Type::Create:
		str = formatString(L".create \"%s\",0x%08X",file->getFileName(),file->getOriginalHeaderSize());
		break;
	case Type::Copy:
		str = formatString(L".open \"%s\",\"%s\",0x%08X",file->getOriginalFileName(),
			file->getFileName(),file->getOriginalHeaderSize());
		break;
	case Type::Close:
		str = L".close";
		break;
	}

	tempData.writeLine(g_fileManager->getVirtualAddress(),str);
}


//
// CDirectivePosition
//

CDirectivePosition::CDirectivePosition(Type type, ArgumentList& Args)
	: type(type)
{
	if (ConvertExpression(Args[0].text,position) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid ram address %s",Args[0].text);
	}
	
	exec();
	Global.Section++;
}

void CDirectivePosition::exec()
{
	switch (type)
	{
	case Physical:
		g_fileManager->seekPhysical((u32)position);
		break;
	case Virtual:
		g_fileManager->seekVirtual((u32)position);
		break;
	}
}

bool CDirectivePosition::Validate()
{
	Arch->NextSection();
	Global.Section++;
	exec();
	return false;
}

void CDirectivePosition::Encode()
{
	Arch->NextSection();
	Global.Section++;
	exec();
}

void CDirectivePosition::writeTempData(TempData& tempData)
{
	switch (type)
	{
	case Physical:
		tempData.writeLine(g_fileManager->getVirtualAddress(),formatString(L".orga 0x%08X",(u32)position));
		break;
	case Virtual:
		tempData.writeLine(g_fileManager->getVirtualAddress(),formatString(L".org 0x%08X",(u32)position));
		break;
	}
}

//
// CDirectiveIncbin
//

CDirectiveIncbin::CDirectiveIncbin(ArgumentList& args)
{
	fileName = getFullPathName(args[0].text);

	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::FatalError,L"File %s not found",fileName);
		return;
	}

	int inputFileSize = fileSize(fileName);
	if (args.size() >= 2)
	{
		// load start address
		if (ConvertExpression(args[1].text,startAddress) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid start address %s",args[1].text);
			return;
		}

		if (startAddress >= inputFileSize)
		{
			Logger::printError(Logger::Error,L"Start address 0x%08X after end of file",startAddress);
			return;
		}

		if (args.size() >= 3)
		{
			// load size too
			if (ConvertExpression(args[2].text,loadSize) == false)
			{
				Logger::printError(Logger::FatalError,L"Invalid size %s",args[1].text);
				return;
			}

			if (startAddress+loadSize > inputFileSize)
			{
				Logger::printError(Logger::Warning,L"Loading beyond file end, truncating");
				loadSize =  inputFileSize-startAddress;
			}
		} else {
			loadSize =  inputFileSize-startAddress;
		}
	} else {
		startAddress = 0;
		loadSize = inputFileSize;
	}

	g_fileManager->advanceMemory(loadSize);
}

bool CDirectiveIncbin::Validate()
{
	Arch->NextSection();
	g_fileManager->advanceMemory(loadSize);
	return false;
}

void CDirectiveIncbin::Encode()
{
	if (loadSize != 0)
	{
		ByteArray data = ByteArray::fromFile(fileName,(u32)startAddress,loadSize);
		if (data.size() == 0)
		{
			Logger::printError(Logger::Error,L"Could not read file \"%s\"",fileName);
			return;
		}
		g_fileManager->write(data.data(),data.size());
	}
}

void CDirectiveIncbin::writeTempData(TempData& tempData)
{
	tempData.writeLine(g_fileManager->getVirtualAddress(),formatString(L".incbin \"%s\"",fileName));
}

void CDirectiveIncbin::writeSymData(SymbolData& symData)
{
	symData.addData(g_fileManager->getVirtualAddress(),loadSize,SymbolData::Data8);
}


//
// CDirectiveAlign
//

CDirectiveAlign::CDirectiveAlign(ArgumentList& args)
{
	if (args.size() >= 1)
	{
		if (ConvertExpression(args[0].text,alignment) == false)
		{
			Logger::printError(Logger::FatalError,L"Invalid alignment %s",args[0].text);
		}
		if (isPowerOfTwo(alignment) == false)
		{
			Logger::printError(Logger::Error,L"Invalid alignment %d",alignment);
		}
	} else {
		alignment = Arch->GetWordSize();
	}

	int num = computePadding();
}

int CDirectiveAlign::computePadding()
{
	unsigned int mod = g_fileManager->getVirtualAddress() % alignment;
	return mod ? alignment-mod : 0;
}

bool CDirectiveAlign::Validate()
{
	Arch->NextSection();
	int num = computePadding();
	g_fileManager->advanceMemory(num);
	return false;
}

void CDirectiveAlign::Encode()
{
	unsigned char AlignBuffer[128];
	
	int n = computePadding();
	memset(AlignBuffer,0,n > 128 ? 128 : n);
	while (n > 128)
	{
		g_fileManager->write(AlignBuffer,128);
		n -= 128;
	}
	g_fileManager->write(AlignBuffer,n);
}

void CDirectiveAlign::writeTempData(TempData& tempData)
{
	tempData.writeLine(g_fileManager->getVirtualAddress(),formatString(L".align 0x%08X",alignment));
}


//
// CDirectiveHeaderSize
//

CDirectiveHeaderSize::CDirectiveHeaderSize(ArgumentList& args)
{
	if (ConvertExpression(args[0].text,headerSize) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid header size %s",args[0].text);
	}

	updateFile();
}

void CDirectiveHeaderSize::updateFile()
{
	AssemblerFile *openFile = g_fileManager->getOpenFile();
	if (!openFile->hasFixedVirtualAddress())
	{
		Logger::printError(Logger::Error,L"Header size not applicable for this file");
		return;
	}
	GenericAssemblerFile* file = static_cast<GenericAssemblerFile*>(openFile);
	file->setHeaderSize((u32)headerSize);
}

bool CDirectiveHeaderSize::Validate()
{
	updateFile();
	return false;
}

void CDirectiveHeaderSize::Encode()
{
	updateFile();
}

void CDirectiveHeaderSize::writeTempData(TempData& tempData)
{
	tempData.writeLine(g_fileManager->getVirtualAddress(),formatString(L".headersize 0x%08X",headerSize));
}


//
// DirectiveObjImport
//

DirectiveObjImport::DirectiveObjImport(ArgumentList& args)
{
	if (rel.init(args[0].text))
	{
		rel.exportSymbols();

		if (args.size() == 2)
			rel.writeCtor(args[1].text);
	}
}

bool DirectiveObjImport::Validate()
{
	u64 memory = g_fileManager->getVirtualAddress();
	rel.relocate(memory);
	g_fileManager->advanceMemory((size_t)memory);
	return rel.hasDataChanged();
}

void DirectiveObjImport::Encode()
{
	ByteArray& data = rel.getData();
	g_fileManager->write(data.data(),data.size());
}

void DirectiveObjImport::writeSymData(SymbolData& symData)
{
	rel.writeSymbols(symData);
}