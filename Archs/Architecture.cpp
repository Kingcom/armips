#include "stdafx.h"
#include "Architecture.h"
#include "Core/Common.h"

CInvalidArchitecture InvalidArchitecture;

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

int CInvalidArchitecture::GetWordSize()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
	return 0;
}

IElfRelocator* CInvalidArchitecture::getElfRelocator()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
	return NULL;
}

