#include "stdafx.h"
#include "Architecture.h"
#include "Core/Common.h"
#include "Core/Directives.h"

CInvalidArchitecture InvalidArchitecture;

void CInvalidArchitecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

bool CInvalidArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	return directiveAssembleGlobal(name,args);
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

