#include "stdafx.h"
#include "ExpressionFunctions.h"
#include "Misc.h"
#include "Common.h"

ExpressionValue expressionFunctionEndianness(const std::vector<ExpressionValue>& parameters)
{
	Endianness endianness = g_fileManager->getEndianness();

	ExpressionValue result;
	result.type = ExpressionValueType::String;

	switch (endianness)
	{
	case Endianness::Little:
		result.strValue = L"little";
		break;
	case Endianness::Big:
		result.strValue = L"big";
		break;
	}

	return result;
}

const ExpressionFunctionMap expressionFunctions = {
	{ L"endianness",	{ &expressionFunctionEndianness,	0,	0 } },
};
