#include "Archs/Architecture.h"

#include "Core/Common.h"
#include "Core/ELF/ElfRelocator.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

CInvalidArchitecture InvalidArchitecture;

void CArchitecture::registerExpressionFunctions([[maybe_unused]] ExpressionFunctionHandler &handler)
{
}

ArchitectureCommand::ArchitectureCommand(const std::wstring& tempText, const std::wstring& symText)
{
	this->tempText = tempText;
	this->symText = symText;
	this->endianness = Arch->getEndianness();
}

bool ArchitectureCommand::Validate(const ValidateState &state)
{
	position = g_fileManager->getVirtualAddress();
	g_fileManager->setEndianness(endianness);
	return false;
}

void ArchitectureCommand::Encode() const
{
	g_fileManager->setEndianness(endianness);
}

void ArchitectureCommand::writeTempData(TempData& tempData) const
{
	if (tempText.size() != 0)
	{
		std::wstringstream stream(tempText);

		std::wstring line;
		while (std::getline(stream,line,L'\n'))
		{
			if (line.size() != 0)
				tempData.writeLine(position,line);
		}
	}
}

void ArchitectureCommand::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (position == -1)
		return;
	
	if (symText.size() != 0)
		symData.addLabel(position,symText);
}


void CInvalidArchitecture::NextSection()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

void CInvalidArchitecture::Pass2()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

void CInvalidArchitecture::Revalidate()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

std::unique_ptr<IElfRelocator> CInvalidArchitecture::getElfRelocator()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
	return nullptr;
}
