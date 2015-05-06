#include "stdafx.h"
#include "Arm.h"
#include "Core/Common.h"
#include "ArmRelocator.h"
#include "ArmParser.h"

CArmArchitecture Arm;

CArmArchitecture::CArmArchitecture()
{
	Pools = NULL;
	clear();
}

CArmArchitecture::~CArmArchitecture()
{
	clear();
}

CAssemblerCommand* CArmArchitecture::parseDirective(Parser& parser)
{
	ArmParser armParser;

	return armParser.parseDirective(parser);
}

CAssemblerCommand* CArmArchitecture::parseOpcode(Parser& parser)
{
	ArmParser armParser;

	if (thumb)
		return armParser.parseThumbOpcode(parser);
	else
		return armParser.parseArmOpcode(parser);
}

bool CArmArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	Logger::printError(Logger::FatalError,L"Unsupported operation");
	return false;
}

void CArmArchitecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	Logger::printError(Logger::FatalError,L"Unsupported operation");
}

void CArmArchitecture::clear()
{
	if (Pools != NULL)
		delete[] Pools;

	thumb = false;
	arm9 = false;
	PoolCount = 0;
	Pools = NULL;
}

void CArmArchitecture::Pass2()
{
	Pools = new ArmPool[PoolCount];
	CurrentPool = 0;
}

void CArmArchitecture::Revalidate()
{
	for (size_t i = 0; i < PoolCount; i++)
	{
		Pools[i].Clear();
	}
	CurrentPool = 0;
}

void CArmArchitecture::NextSection()
{

}

IElfRelocator* CArmArchitecture::getElfRelocator()
{
	return new ArmElfRelocator(arm9);
}

int CArmArchitecture::AddToCurrentPool(int value)
{
	if (CurrentPool == PoolCount)
	{
		return -1;
	}
	return (int) Pools[CurrentPool].AddEntry(value);
};
