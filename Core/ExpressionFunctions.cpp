#include "stdafx.h"
#include "ExpressionFunctions.h"
#include "Misc.h"
#include "Common.h"
#include <regex>
#include "../Archs/ARM/Arm.h"

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, u64& dest,
	const std::wstring& funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || parameters[index].isInt() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter %d for %s: expecting integer",index+1,funcName);
		return false;
	}

	dest = parameters[index].intValue;
	return true;
}

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || parameters[index].isString() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter %d for %s: expecting string",index+1,funcName);
		return false;
	}

	dest = &parameters[index].strValue;
	return true;
}

#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();
#define GET_OPTIONAL_PARAM(params,index,dest,defaultValue) \
	dest = defaultValue; \
	if (getExpFuncParameter(params,index,dest,funcName,true) == false) \
		return ExpressionValue();


ExpressionValue expFuncVersion(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	u64 value = ARMIPS_VERSION_MAJOR*100 + ARMIPS_VERSION_MINOR*10 + ARMIPS_VERSION_REVISION;
	return ExpressionValue(value);
}

ExpressionValue expFuncEndianness(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	result.type = ExpressionValueType::String;

	switch (g_fileManager->getEndianness())
	{
	case Endianness::Little:
		return ExpressionValue(L"little");
	case Endianness::Big:
		return ExpressionValue(L"big");
	}

	return ExpressionValue();
}

ExpressionValue expFuncFileExists(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	GET_PARAM(parameters,0,fileName);

	std::wstring fullName = getFullPathName(*fileName);
	return ExpressionValue(fileExists(fullName) ? UINT64_C(1) : UINT64_C(0));
}

ExpressionValue expFuncFileSize(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	GET_PARAM(parameters,0,fileName);

	std::wstring fullName = getFullPathName(*fileName);
	return ExpressionValue(fileSize(fullName));
}

ExpressionValue expFuncToString(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
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

ExpressionValue expFuncToHex(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	u64 value, digits;
	GET_PARAM(parameters,0,value);
	GET_OPTIONAL_PARAM(parameters,1,digits,8);

	return ExpressionValue(formatString(L"%0*X",digits,value));
}

ExpressionValue expFuncStrlen(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	GET_PARAM(parameters,0,source);

	return ExpressionValue((u64)source->size());
}

ExpressionValue expFuncSubstr(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	u64 start, count;
	const std::wstring* source;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,start);
	GET_PARAM(parameters,2,count);

	return ExpressionValue(source->substr((size_t)start,(size_t)count));
}

ExpressionValue expFuncRegExMatch(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

	try
	{
		std::wregex regex(*regexString);
		bool found = std::regex_match(*source,regex);
		return ExpressionValue(found ? UINT64_C(1) : UINT64_C(0));
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
}

ExpressionValue expFuncRegExSearch(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

	try
	{
		std::wregex regex(*regexString);
		bool found = std::regex_search(*source,regex);
		return ExpressionValue(found ? UINT64_C(1) : UINT64_C(0));
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
}

ExpressionValue expFuncRegExExtract(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;
	u64 matchIndex;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);
	GET_OPTIONAL_PARAM(parameters,2,matchIndex,0);

	try
	{
		std::wregex regex(*regexString);
		std::wsmatch result;
		bool found = std::regex_search(*source,result,regex);
		if (found == false || matchIndex >= result.size())
		{
			Logger::queueError(Logger::Error,L"Capture group index %d does not exist",matchIndex);
			return ExpressionValue();
		}
	
		return ExpressionValue(result[(size_t)matchIndex].str());
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
}

ExpressionValue expFuncFind(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	u64 start;
	const std::wstring* source;
	const std::wstring* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,0);

	size_t pos = source->find(*value,(size_t)start);
	return ExpressionValue(pos == std::wstring::npos ? (u64) -1 : pos);
}

ExpressionValue expFuncRFind(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	u64 start;
	const std::wstring* source;
	const std::wstring* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,std::wstring::npos);

	size_t pos = source->rfind(*value,(size_t)start);
	return ExpressionValue(pos == std::wstring::npos ? (u64) -1 : pos);
}


template<typename T>
ExpressionValue expFuncRead(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	u64 pos;

	GET_PARAM(parameters,0,fileName);
	GET_OPTIONAL_PARAM(parameters,1,pos,0);

	std::wstring fullName = getFullPathName(*fileName);

	BinaryFile file;
	if (file.open(fullName,BinaryFile::Read) == false)
	{
		Logger::queueError(Logger::Error,L"Could not open %s",fileName);
		return ExpressionValue();
	}

	file.setPos((long)pos);

	T buffer;
	if (file.read(&buffer,sizeof(T)) != sizeof(T))
		return ExpressionValue();

	return ExpressionValue((u64) buffer);
}

ExpressionValue expFuncDefined(ExpressionInternal* exp)
{
	if (exp == nullptr || exp->isIdentifier() == false)
	{
		Logger::queueError(Logger::Error,L"Invalid parameter 1 for defined: expecting identifier");
		return ExpressionValue();
	}

	const std::wstring& name = exp->getStringValue();
	Label* label = Global.symbolTable.getLabel(name,exp->getFileNum(),exp->getSection());

	if (label == nullptr)
		return ExpressionValue();

	return ExpressionValue(label->isDefined() ? UINT64_C(1) : UINT64_C(0)); 
}

ExpressionValue expFuncIsArm(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	bool isArm = Arch == &Arm && Arm.GetThumbMode() == false;
	return ExpressionValue(isArm ? UINT64_C(1) : UINT64_C(0)); 
}

ExpressionValue expFuncIsThumb(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	bool isThumb = Arch == &Arm && Arm.GetThumbMode() == true;
	return ExpressionValue(isThumb ? UINT64_C(1) : UINT64_C(0)); 
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
	{ L"regex_match",	{ &expFuncRegExMatch,	2,	2,	true } },
	{ L"regex_search",	{ &expFuncRegExSearch,	2,	2,	true } },
	{ L"regex_extract",	{ &expFuncRegExExtract,	2,	3,	true } },
	{ L"find",			{ &expFuncFind,			2,	3,	true } },
	{ L"rfind",			{ &expFuncRFind,		2,	3,	true } },

	{ L"readbyte",		{ &expFuncRead<u8>,		1,	2,	true } },
	{ L"readu8",		{ &expFuncRead<u8>,		1,	2,	true } },
	{ L"readu16",		{ &expFuncRead<u16>,	1,	2,	true } },
	{ L"readu32",		{ &expFuncRead<u32>,	1,	2,	true } },

	{ L"isarm",			{ &expFuncIsArm,		0,	0,	true } },
	{ L"isthumb",		{ &expFuncIsThumb,		0,	0,	true } },

};
