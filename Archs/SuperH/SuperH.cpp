#include "Archs/SuperH/SuperH.h"

#include "Archs/SuperH/ShElfRelocator.h"
#include "Archs/SuperH/ShExpressionFunctions.h"
#include "Archs/SuperH/ShParser.h"

CShArchitecture SuperH;

CShArchitecture::CShArchitecture()
{
	version = SHARCH_INVALID;
}

std::unique_ptr<CAssemblerCommand> CShArchitecture::parseDirective(Parser& parser)
{
	ShParser shParser;
	return shParser.parseDirective(parser);
}

std::unique_ptr<CAssemblerCommand> CShArchitecture::parseOpcode(Parser& parser)
{
	ShParser shParser;
	return shParser.parseOpcode(parser);
}

void CShArchitecture::registerExpressionFunctions(ExpressionFunctionHandler &handler)
{
	registerShExpressionFunctions(handler);
}

void CShArchitecture::NextSection()
{
}

void CShArchitecture::Revalidate()
{
}


std::unique_ptr<IElfRelocator> CShArchitecture::getElfRelocator()
{
	switch (version)
	{
	case SHARCH_SATURN:
		return std::make_unique<ShElfRelocator>();
	default:
		return nullptr;
	}
}
