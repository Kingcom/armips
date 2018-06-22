#include "stdafx.h"
#include "Arm.h"
#include "Core/Common.h"
#include "ArmElfRelocator.h"
#include "ArmParser.h"
#include "ArmExpressionFunctions.h"

CArmArchitecture Arm;

CArmArchitecture::CArmArchitecture()
{
	clear();
}

CArmArchitecture::~CArmArchitecture()
{
	clear();
}

std::unique_ptr<CAssemblerCommand> CArmArchitecture::parseDirective(Parser& parser)
{
	ArmParser armParser;

	return armParser.parseDirective(parser);
}

std::unique_ptr<CAssemblerCommand> CArmArchitecture::parseOpcode(Parser& parser)
{
	ArmParser armParser;

	if (thumb)
		return armParser.parseThumbOpcode(parser);
	else
		return armParser.parseArmOpcode(parser);
}

const ExpressionFunctionMap& CArmArchitecture::getExpressionFunctions()
{
	return armExpressionFunctions;
}

void CArmArchitecture::clear()
{
	currentPoolContent.clear();
	thumb = false;
}

void CArmArchitecture::Pass2()
{
	currentPoolContent.clear();
}

void CArmArchitecture::Revalidate()
{
	for (ArmPoolEntry& entry: currentPoolContent)
	{
		entry.command->applyFileInfo();
		Logger::queueError(Logger::Error,L"Unable to find literal pool");
	}

	currentPoolContent.clear();
}

void CArmArchitecture::NextSection()
{

}

std::unique_ptr<IElfRelocator> CArmArchitecture::getElfRelocator()
{
	return make_unique<ArmElfRelocator>(version != AARCH_GBA);
}

void CArmArchitecture::addPoolValue(ArmOpcodeCommand* command, int32_t value)
{
	ArmPoolEntry entry;
	entry.command = command;
	entry.value = value;

	currentPoolContent.push_back(entry);
}
