#include "Commands/CDirectiveFile.h"

#include "Archs/Architecture.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"
#include "Util/FileClasses.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include <cstring>

//
// CDirectiveFile
//

CDirectiveFile::CDirectiveFile()
{
	type = Type::Invalid;
	file = nullptr;
}

void CDirectiveFile::initOpen(const fs::path& fileName, int64_t memory)
{
	type = Type::Open;
	fs::path fullName = getFullPathName(fileName);

	file = std::make_shared<GenericAssemblerFile>(fullName,memory,false);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initCreate(const fs::path& fileName, int64_t memory)
{
	type = Type::Create;
	fs::path fullName = getFullPathName(fileName);

	file = std::make_shared<GenericAssemblerFile>(fullName,memory,true);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initCopy(const fs::path& inputName, const fs::path& outputName, int64_t memory)
{
	type = Type::Copy;
	fs::path fullInputName = getFullPathName(inputName);
	fs::path fullOutputName = getFullPathName(outputName);
	
	file = std::make_shared<GenericAssemblerFile>(fullOutputName,fullInputName,memory);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initClose()
{
	type = Type::Close;
	updateSection(++Global.Section);
}

bool CDirectiveFile::Validate(const ValidateState &state)
{
	if (state.noFileChange)
	{
		if (type == Type::Close)
			Logger::queueError(Logger::Error, "Cannot close file within %S", state.noFileChangeDirective);
		else
			Logger::queueError(Logger::Error, "Cannot open new file within %S", state.noFileChangeDirective);
		return false;
	}

	virtualAddress = g_fileManager->getVirtualAddress();
	Architecture::current().NextSection();

	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		g_fileManager->openFile(file,true);
		return false;
	case Type::Close:
		closeFile = g_fileManager->getOpenFile();
		g_fileManager->closeFile();
		return false;
	case Type::Invalid:
		break;
	}
	
	return false;
}

void CDirectiveFile::Encode() const
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
	case Type::Invalid:
		// TODO: Assert?
		break;
	}
}

void CDirectiveFile::writeTempData(TempData& tempData) const
{
	std::string str;

	switch (type)
	{
	case Type::Open:
		str = tfm::format(".open \"%s\",0x%08X",file->getFileName().u8string(),file->getOriginalHeaderSize());
		break;
	case Type::Create:
		str = tfm::format(".create \"%s\",0x%08X",file->getFileName().u8string(),file->getOriginalHeaderSize());
		break;
	case Type::Copy:
		str = tfm::format(".open \"%s\",\"%s\",0x%08X",file->getOriginalFileName().u8string(),
			file->getFileName().u8string(),file->getOriginalHeaderSize());
		break;
	case Type::Close:
		str = ".close";
		break;
	case Type::Invalid:
		// TODO: Assert?
		break;
	}

	tempData.writeLine(virtualAddress,str);
}

void CDirectiveFile::writeSymData(SymbolData& symData) const
{
	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		file->beginSymData(symData);
		break;
	case Type::Close:
		if (closeFile)
			closeFile->endSymData(symData);
		break;
	case Type::Invalid:
		// TODO: Assert?
		break;
	}
}

//
// CDirectivePosition
//

CDirectivePosition::CDirectivePosition(Expression expression, Type type)
	: expression(expression), type(type)
{
	updateSection(++Global.Section);
}

void CDirectivePosition::exec() const
{
	switch (type)
	{
	case Physical:
		g_fileManager->seekPhysical(position);
		break;
	case Virtual:
		g_fileManager->seekVirtual(position);
		break;
	}
}

bool CDirectivePosition::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (!expression.evaluateInteger(position))
	{
		Logger::queueError(Logger::FatalError, "Invalid position");
		return false;
	}

	Architecture::current().NextSection();
	exec();
	return false;
}

void CDirectivePosition::Encode() const
{
	Architecture::current().NextSection();
	exec();
}

void CDirectivePosition::writeTempData(TempData& tempData) const
{
	switch (type)
	{
	case Physical:
		tempData.writeLine(virtualAddress,tfm::format(".orga 0x%08X",position));
		break;
	case Virtual:
		tempData.writeLine(virtualAddress,tfm::format(".org 0x%08X",position));
		break;
	}
}

//
// CDirectiveIncbin
//

CDirectiveIncbin::CDirectiveIncbin(const fs::path& fileName)
	: size(0), start(0)
{
	this->fileName = getFullPathName(fileName);

	if (!fs::exists(this->fileName))
	{
		Logger::printError(Logger::FatalError, "File %s not found",this->fileName.u8string());
	}

	std::error_code error;
	this->fileSize = static_cast<int64_t>(fs::file_size(this->fileName, error));
}

bool CDirectiveIncbin::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (startExpression.isLoaded())
	{
		if (!startExpression.evaluateInteger(start))
		{
			Logger::queueError(Logger::Error, "Invalid position expression");
			return false;
		}

		if (start > fileSize)
		{
			Logger::queueError(Logger::Error, "Start position past end of file");
			return false;
		}
	} else {
		start = 0;
	}

	if (sizeExpression.isLoaded())
	{
		if (!sizeExpression.evaluateInteger(size))
		{
			Logger::queueError(Logger::Error, "Invalid size expression");
			return false;
		}
	} else {
		size = fileSize-start;
	}

	if (start+size > fileSize)
	{
		Logger::queueError(Logger::Warning, "Read size truncated due to file size");
		size = fileSize-start;
	}

	Architecture::current().NextSection();
	g_fileManager->advanceMemory(size);
	return false;
}

void CDirectiveIncbin::Encode() const
{
	if (size != 0)
	{
		ByteArray data = ByteArray::fromFile(fileName,(long)start,size);
		if ((int) data.size() != size)
		{
			Logger::printError(Logger::Error, "Could not read file \"%s\"",fileName.u8string());
			return;
		}
		g_fileManager->write(data.data(),data.size());
	}
}

void CDirectiveIncbin::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(".incbin \"%s\"",fileName.u8string()));
}

void CDirectiveIncbin::writeSymData(SymbolData& symData) const
{
	symData.addData(virtualAddress,size,SymbolData::Data8);
}


//
// CDirectiveAlignFill
//

CDirectiveAlignFill::CDirectiveAlignFill(int64_t value, Mode mode)
{
	this->mode = mode;
	this->value = value;
	this->finalSize = 0;
	this->fillByte = 0;
}

CDirectiveAlignFill::CDirectiveAlignFill(Expression& value, Mode mode)
	: CDirectiveAlignFill(0,mode)
{
	valueExpression = value;
}

CDirectiveAlignFill::CDirectiveAlignFill(Expression& value, Expression& fillValue, Mode mode)
	: CDirectiveAlignFill(value,mode)
{
	fillExpression = fillValue;
}

bool CDirectiveAlignFill::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (valueExpression.isLoaded())
	{
		if (!valueExpression.evaluateInteger(value))
		{
			Logger::queueError(Logger::FatalError, "Invalid %s",mode == Fill ? "size" : "alignment");
			return false;
		}
	}

	if (mode != Fill && !isPowerOfTwo(value))
	{
		Logger::queueError(Logger::Error, "Invalid alignment %d", value);
		return false;
	}

	int64_t oldSize = finalSize;
	int64_t mod;
	switch (mode)
	{
	case AlignVirtual:
		mod = g_fileManager->getVirtualAddress() % value;
		finalSize = mod ? value-mod : 0;
		break;
	case AlignPhysical:
		mod = g_fileManager->getPhysicalAddress() % value;
		finalSize = mod ? value-mod : 0;
		break;
	case Fill:
		finalSize = value;
		break;
	}

	if (fillExpression.isLoaded())
	{
		if (!fillExpression.evaluateInteger(fillByte))
		{
			Logger::printError(Logger::FatalError, "Invalid fill value");
			return false;
		}
	}

	Architecture::current().NextSection();
	g_fileManager->advanceMemory(finalSize);

	bool result = oldSize != finalSize;
	oldSize = finalSize;
	return result;
}

void CDirectiveAlignFill::Encode() const
{
	unsigned char buffer[128];
	int64_t n = finalSize;

	memset(buffer,fillByte,n > 128 ? 128 : n);
	while (n > 128)
	{
		g_fileManager->write(buffer,128);
		n -= 128;
	}

	g_fileManager->write(buffer,n);
}

void CDirectiveAlignFill::writeTempData(TempData& tempData) const
{
	switch (mode)
	{
	case AlignVirtual:
		tempData.writeLine(virtualAddress,tfm::format(".align 0x%08X",value));
		break;
	case AlignPhysical:
		tempData.writeLine(virtualAddress, tfm::format(".aligna 0x%08X", value));
		break;
	case Fill:
		tempData.writeLine(virtualAddress,tfm::format(".fill 0x%08X,0x%02X",value,fillByte));
		break;
	}
}

void CDirectiveAlignFill::writeSymData(SymbolData& symData) const
{
	switch (mode)
	{
	case AlignVirtual:	// ?
	case AlignPhysical:	// ?
		break;
	case Fill:
		symData.addData(virtualAddress,value,SymbolData::Data8);
		break;
	}
}

//
// CDirectiveSkip
//

CDirectiveSkip::CDirectiveSkip(Expression& expression)
	: expression(expression) {}

bool CDirectiveSkip::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (expression.isLoaded())
	{
		if (!expression.evaluateInteger(value))
		{
			Logger::queueError(Logger::FatalError, "Invalid skip length");
			return false;
		}
	}

	Architecture::current().NextSection();
	g_fileManager->advanceMemory(value);

	return false;
}

void CDirectiveSkip::Encode() const
{
	Architecture::current().NextSection();
	g_fileManager->advanceMemory(value);
}

void CDirectiveSkip::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(".skip 0x%08X",value));
}

//
// CDirectiveHeaderSize
//

CDirectiveHeaderSize::CDirectiveHeaderSize(Expression expression)
	: expression(expression) {}

void CDirectiveHeaderSize::exec() const
{
	std::shared_ptr<AssemblerFile> openFile = g_fileManager->getOpenFile();
	if (!openFile->hasFixedVirtualAddress())
	{
		Logger::printError(Logger::Error, "Header size not applicable for this file");
		return;
	}
	std::shared_ptr<GenericAssemblerFile> file = std::static_pointer_cast<GenericAssemblerFile>(openFile);
	int64_t physicalAddress = file->getPhysicalAddress();
	file->setHeaderSize(headerSize);
	file->seekPhysical(physicalAddress);
}

bool CDirectiveHeaderSize::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (!expression.evaluateInteger(headerSize))
	{
		Logger::queueError(Logger::FatalError, "Invalid header size");
		return false;
	}

	exec();
	return false;
}

void CDirectiveHeaderSize::Encode() const
{
	exec();
}

void CDirectiveHeaderSize::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(".headersize %s0x%08X",
		headerSize < 0 ? "-" : "", headerSize < 0 ? -headerSize : headerSize));
}


//
// DirectiveObjImport
//

DirectiveObjImport::DirectiveObjImport(const fs::path& inputName)
{
	ctor = nullptr;
	success = rel.init(inputName);
	if (success)
		rel.exportSymbols();
}

DirectiveObjImport::DirectiveObjImport(const fs::path& inputName, const Identifier& ctorName)
{
	success = rel.init(inputName);
	if (success)
	{
		rel.exportSymbols();
		ctor = rel.generateCtor(ctorName);
	}
}

bool DirectiveObjImport::Validate(const ValidateState &state)
{
	bool result = false;
	if (ctor != nullptr && ctor->Validate(state))
		result = true;

	int64_t memory = g_fileManager->getVirtualAddress();
	rel.relocate(memory);
	g_fileManager->advanceMemory((size_t)memory);

	return rel.hasDataChanged() || result;
}

void DirectiveObjImport::Encode() const
{
	if (ctor != nullptr)
		ctor->Encode();

	const ByteArray& data = rel.getData();
	g_fileManager->write(data.data(),data.size());
}

void DirectiveObjImport::writeTempData(TempData& tempData) const
{
	if (ctor != nullptr)
		ctor->writeTempData(tempData);
}

void DirectiveObjImport::writeSymData(SymbolData& symData) const
{
	if (ctor != nullptr)
		ctor->writeSymData(symData);

	rel.writeSymbols(symData);
}
