#include "stdafx.h"
#include "ExpressionFunctions.h"
#include "Misc.h"
#include "Common.h"

ExpressionValue expFuncEndianness(const std::vector<ExpressionValue>& parameters)
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

ExpressionValue expFuncFileExists(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	std::wstring fileName = getFullPathName(parameters[0].strValue);

	result.type = ExpressionValueType::Integer;
	result.intValue = fileExists(fileName) ? 1 : 0;
	return result;
}

ExpressionValue expFuncFileSize(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	std::wstring fileName = getFullPathName(parameters[0].strValue);

	result.type = ExpressionValueType::Integer;
	result.intValue = fileSize(fileName);
	return result;
}

const ExpressionFunctionMap expressionFunctions = {
	{ L"endianness",	{ &expFuncEndianness,	0,	0 } },
	{ L"fileexists",	{ &expFuncFileExists,	1,	1 } },
	{ L"filesize",		{ &expFuncFileSize,		1,	1 } },
};
