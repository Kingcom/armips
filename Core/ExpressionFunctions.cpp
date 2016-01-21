#include "stdafx.h"
#include "ExpressionFunctions.h"
#include "Misc.h"
#include "Common.h"

ExpressionValue expFuncVersion(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;
	result.intValue = ARMIPS_VERSION_MAJOR*100 + ARMIPS_VERSION_MINOR*10 + ARMIPS_VERSION_REVISION;
	return result;
}

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

ExpressionValue expFuncToString(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::String:
		result.strValue = parameters[0].strValue;
		break;
	case ExpressionValueType::Integer:
		result.strValue = formatString(L"%d",parameters[0].intValue);
		break;
	case ExpressionValueType::Float:
		result.strValue = formatString(L"%g",parameters[0].floatValue);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::String;
	return result;
}

ExpressionValue expFuncToHex(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isInt() == false || parameters.size() >= 2 && parameters[1].isInt() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	u32 digits = 8;
	if (parameters.size() >= 2)
		digits = (u32) parameters[1].intValue;

	result.type = ExpressionValueType::String;
	result.strValue = formatString(L"%0*X",digits,parameters[0].intValue);
	return result;
}

ExpressionValue expFuncStrlen(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}


	result.type = ExpressionValueType::Integer;
	result.intValue = parameters[0].strValue.size();
	return result;
}

ExpressionValue expFuncSubstr(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false || parameters[1].isInt() == false
		|| parameters[2].isInt() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	size_t start = (size_t) parameters[1].intValue;
	size_t count = (size_t) parameters[2].intValue;

	result.type = ExpressionValueType::String;
	result.strValue = parameters[0].strValue.substr(start,count);
	return result;
}

ExpressionValue expFuncFind(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false || parameters[1].isString() == false
		|| (parameters.size() >= 3 && parameters[2].isInt() == false))
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	size_t start = parameters.size() >= 3 ? (size_t) parameters[2].intValue : 0;
	size_t pos = parameters[0].strValue.find(parameters[1].strValue,start);

	result.type = ExpressionValueType::Integer;
	result.intValue = pos == std::wstring::npos ? (u64) -1 : pos;

	return result;
}

ExpressionValue expFuncRFind(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	if (parameters[0].isString() == false || parameters[1].isString() == false
		|| (parameters.size() >= 3 && parameters[2].isInt() == false))
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	size_t start = parameters.size() >= 3 ? (size_t) parameters[2].intValue : std::wstring::npos;
	size_t pos = parameters[0].strValue.rfind(parameters[1].strValue,start);

	result.type = ExpressionValueType::Integer;
	result.intValue = pos == std::wstring::npos ? (u64) -1 : pos;

	return result;
}


template<typename T>
ExpressionValue expFuncRead(const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	T buffer;

	if (parameters[0].isString() == false || (parameters.size() >= 2 && parameters[1].isInt() == false))
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	std::wstring fileName = getFullPathName(parameters[0].strValue);
	u64 pos = parameters.size() >= 2 ? parameters[1].intValue : 0;

	BinaryFile file;
	if (file.open(fileName,BinaryFile::Read) == false)
	{
		Logger::queueError(Logger::Error,L"Could not open %s",fileName);
		return result;
	}

	file.setPos((long)pos);

	if (file.read(&buffer,sizeof(T)) == sizeof(T))
	{
		result.type = ExpressionValueType::Integer;
		result.intValue = (u64) buffer;
	}

	return result;
}

ExpressionValue expFuncDefined(ExpressionInternal* exp)
{
	ExpressionValue result;

	if (exp == nullptr || exp->isIdentifier() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter");
		return result;
	}

	const std::wstring& name = exp->getStringValue();
	Label* label = Global.symbolTable.getLabel(name,exp->getFileNum(),exp->getSection());

	if (label != nullptr)
	{
		result.type = ExpressionValueType::Integer;
		result.intValue = label->isDefined();
	}

	return result;
}

const ExpressionFunctionMap expressionFunctions = {
	{ L"version",		{ &expFuncVersion,		0,	0,	true } },
	{ L"endianness",	{ &expFuncEndianness,	0,	0,	false } },
	{ L"fileexists",	{ &expFuncFileExists,	1,	1,	true } },
	{ L"filesize",		{ &expFuncFileSize,		1,	1,	true } },
	{ L"tostring",		{ &expFuncToString,		1,	1,	true } },
	{ L"tohex",			{ &expFuncToHex,		1,	2,	true } },
	{ L"strlen",		{ &expFuncStrlen,		1,	1,	true } },
	{ L"substr",		{ &expFuncSubstr,		3,	3,	true } },
	{ L"find",			{ &expFuncFind,			2,	3,	true } },
	{ L"rfind",			{ &expFuncRFind,		2,	3,	true } },

	{ L"readbyte",		{ &expFuncRead<u8>,		1,	2,	true } },
	{ L"readu8",		{ &expFuncRead<u8>,		1,	2,	true } },
	{ L"readu16",		{ &expFuncRead<u16>,	1,	2,	true } },
	{ L"readu32",		{ &expFuncRead<u32>,	1,	2,	true } },
};
