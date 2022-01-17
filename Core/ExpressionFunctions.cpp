#include "Core/ExpressionFunctions.h"

#include "Core/Assembler.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include <cmath>

#if ARMIPS_REGEXP
#include <regex>
#endif

#if defined(__clang__)
#if __has_feature(cxx_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif
#elif defined(_MSC_VER) && defined(_CPPUNWIND)
#define ARMIPS_EXCEPTIONS 1
#elif defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const Identifier &funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || !parameters[index].isInt())
	{
		Logger::queueError(Logger::Error, "Invalid parameter %d for %s: expecting integer",index+1,funcName);
		return false;
	}

	dest = parameters[index].intValue;
	return true;
}

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const StringLiteral*& dest,
	const Identifier &funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || !parameters[index].isString())
	{
		Logger::queueError(Logger::Error, "Invalid parameter %d for %s: expecting string",index+1,funcName);
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


ExpressionValue expFuncVersion(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value = ARMIPS_VERSION_MAJOR*1000 + ARMIPS_VERSION_MINOR*10 + ARMIPS_VERSION_REVISION;
	return ExpressionValue(value);
}

ExpressionValue expFuncEndianness(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	result.type = ExpressionValueType::String;

	switch (g_fileManager->getEndianness())
	{
	case Endianness::Little:
		return ExpressionValue(StringLiteral("little"));
	case Endianness::Big:
		return ExpressionValue(StringLiteral("big"));
	}

	return ExpressionValue();
}

ExpressionValue expFuncOutputName(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	std::shared_ptr<AssemblerFile> file = g_fileManager->getOpenFile();
	if (file == nullptr)
	{
		Logger::queueError(Logger::Error, "outputName: no file opened");
		return ExpressionValue();
	}

	std::string value = file->getFileName().u8string();
	return ExpressionValue(value);
}

ExpressionValue expFuncFileExists(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* fileName;
	GET_PARAM(parameters,0,fileName);

	auto fullName = getFullPathName(fileName->path());
	return ExpressionValue(fs::exists(fullName) ? INT64_C(1) : INT64_C(0));
}

ExpressionValue expFuncFileSize(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* fileName;
	GET_PARAM(parameters,0,fileName);

	auto fullName = getFullPathName(fileName->path());

	std::error_code error;
	return ExpressionValue(static_cast<int64_t>(fs::file_size(fullName, error)));
}

ExpressionValue expFuncToString(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::String:
		result.strValue = parameters[0].strValue;
		break;
	case ExpressionValueType::Integer:
		result.strValue = tfm::format("%d",parameters[0].intValue);
		break;
	case ExpressionValueType::Float:
		result.strValue = tfm::format("%#.17g",parameters[0].floatValue);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::String;
	return result;
}

ExpressionValue expFuncToHex(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value, digits;
	GET_PARAM(parameters,0,value);
	GET_OPTIONAL_PARAM(parameters,1,digits,8);

	return ExpressionValue(tfm::format("%0*X",digits,value));
}

ExpressionValue expFuncInt(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.intValue = parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.intValue = (int64_t) parameters[0].floatValue;
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Integer;
	return result;
}

ExpressionValue expFuncRound(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.intValue = parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.intValue = llround(parameters[0].floatValue);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Integer;
	return result;
}

ExpressionValue expFuncFloat(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.floatValue = (double) parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.floatValue = parameters[0].floatValue;
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Float;
	return result;
}

ExpressionValue expFuncFrac(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double intPart;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Float:
		result.floatValue = modf(parameters[0].floatValue,&intPart);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Float;
	return result;
}

ExpressionValue expFuncMin(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double floatMin, floatCur;
	int64_t intMin, intCur;

	floatCur = floatMin = std::numeric_limits<double>::max();
	intCur = intMin = std::numeric_limits<int64_t>::max();
	bool isInt = true;

	for (size_t i = 0; i < parameters.size(); i++)
	{
		switch (parameters[i].type)
		{
		case ExpressionValueType::Integer:
			intCur = parameters[i].intValue;
			floatCur = (double)parameters[i].intValue;
			break;
		case ExpressionValueType::Float:
			floatCur = parameters[i].floatValue;
			isInt = false;
			break;
		default:
			return result;
		}

		if (intCur < intMin)
			intMin = intCur;
		if (floatCur < floatMin)
			floatMin = floatCur;
	}

	if (isInt)
	{
		result.intValue = intMin;
		result.type = ExpressionValueType::Integer;
	}
	else
	{
		result.floatValue = floatMin;
		result.type = ExpressionValueType::Float;
	}

	return result;
}

ExpressionValue expFuncMax(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double floatMax, floatCur;
	int64_t intMax, intCur;

	floatCur = floatMax = std::numeric_limits<double>::min();
	intCur = intMax = std::numeric_limits<int64_t>::min();
	bool isInt = true;

	for (size_t i = 0; i < parameters.size(); i++)
	{
		switch (parameters[i].type)
		{
		case ExpressionValueType::Integer:
			intCur = parameters[i].intValue;
			floatCur = (double)parameters[i].intValue;
			break;
		case ExpressionValueType::Float:
			floatCur = parameters[i].floatValue;
			isInt = false;
			break;
		default:
			return result;
		}

		if (intCur > intMax)
			intMax = intCur;
		if (floatCur > floatMax)
			floatMax = floatCur;
	}

	if (isInt)
	{
		result.intValue = intMax;
		result.type = ExpressionValueType::Integer;
	}
	else
	{
		result.floatValue = floatMax;
		result.type = ExpressionValueType::Float;
	}

	return result;
}

ExpressionValue expFuncAbs(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Float:
		result.type = ExpressionValueType::Float;
		result.floatValue = fabs(parameters[0].floatValue);
		break;
	case ExpressionValueType::Integer:
		result.type = ExpressionValueType::Integer;
		result.intValue = parameters[0].intValue >= 0 ?
			parameters[0].intValue : -parameters[0].intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue expFuncStrlen(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* source;
	GET_PARAM(parameters,0,source);

	return ExpressionValue((int64_t)source->size());
}

ExpressionValue expFuncSubstr(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start, count;
	const StringLiteral* source;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,start);
	GET_PARAM(parameters,2,count);

	return ExpressionValue(source->string().substr((size_t)start,(size_t)count));
}

#if ARMIPS_REGEXP
ExpressionValue expFuncRegExMatch(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* source;
	const StringLiteral* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::regex regex(regexString->string());
		bool found = std::regex_match(source->string(),regex);
		return ExpressionValue(found ? INT64_C(1) : INT64_C(0));
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error, "Invalid regular expression");
		return ExpressionValue();
	}
#endif
}

ExpressionValue expFuncRegExSearch(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* source;
	const StringLiteral* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::regex regex(regexString->string());
		bool found = std::regex_search(source->string(),regex);
		return ExpressionValue(found ? INT64_C(1) : INT64_C(0));
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error, "Invalid regular expression");
		return ExpressionValue();
	}
#endif
}

ExpressionValue expFuncRegExExtract(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* source;
	const StringLiteral* regexString;
	int64_t matchIndex;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);
	GET_OPTIONAL_PARAM(parameters,2,matchIndex,0);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::regex regex(regexString->string());
		std::smatch result;
		bool found = std::regex_search(source->string(),result,regex);
		if (!found || (size_t)matchIndex >= result.size())
		{
			Logger::queueError(Logger::Error, "Capture group index %d does not exist",matchIndex);
			return ExpressionValue();
		}
	
		return ExpressionValue(result[(size_t)matchIndex].str());
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error, "Invalid regular expression");
		return ExpressionValue();
	}
#endif
}
#endif

ExpressionValue expFuncFind(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start;
	const StringLiteral* source;
	const StringLiteral* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,0);

	size_t pos = source->string().find(value->string(),(size_t)start);
	return ExpressionValue(pos == std::string::npos ? INT64_C(-1) : (int64_t) pos);
}

ExpressionValue expFuncRFind(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start;
	const StringLiteral* source;
	const StringLiteral* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,std::string::npos);

	size_t pos = source->string().rfind(value->string(),(size_t)start);
	return ExpressionValue(pos == std::string::npos ? INT64_C(-1) : (int64_t) pos);
}


template<typename T>
ExpressionValue expFuncRead(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* fileName;
	int64_t pos;

	GET_PARAM(parameters,0,fileName);
	GET_OPTIONAL_PARAM(parameters,1,pos,0);

	auto fullName = getFullPathName(fileName->path());

	fs::ifstream file(fullName, fs::ifstream::in | fs::ifstream::binary);
	if (!file.is_open())
	{
		Logger::queueError(Logger::Error, "Could not open %s",*fileName);
		return ExpressionValue();
	}

	file.seekg(pos);
	if (file.eof() || file.fail())
	{
		Logger::queueError(Logger::Error, "Invalid offset 0x%08X of %s", pos, *fileName);
		return ExpressionValue();
	}

	T buffer;
	file.read(reinterpret_cast<char*>(&buffer), sizeof(T));

	if (file.fail())
	{
		Logger::queueError(Logger::Error, "Failed to read %d byte(s) from offset 0x%08X of %s", sizeof(T), pos, *fileName);
		return ExpressionValue();
	}

	return ExpressionValue((int64_t) buffer);
}

ExpressionValue expFuncReadAscii(const Identifier &funcName, const std::vector<ExpressionValue>& parameters)
{
	const StringLiteral* fileName;
	int64_t start;
	int64_t length;

	GET_PARAM(parameters,0,fileName);
	GET_OPTIONAL_PARAM(parameters,1,start,0);
	GET_OPTIONAL_PARAM(parameters,2,length,0);

	auto fullName = getFullPathName(fileName->path());

	std::error_code error;
	int64_t totalSize = static_cast<int64_t>(fs::file_size(fullName, error));

	if (length == 0 || start+length > totalSize)
		length = totalSize-start;

	fs::ifstream file(fullName, fs::ifstream::in | fs::ifstream::binary);
	if (!file.is_open())
	{
		Logger::queueError(Logger::Error, "Could not open %s", *fileName);
		return ExpressionValue();
	}

	file.seekg(start);
	if (file.eof() || file.fail())
	{
		Logger::queueError(Logger::Error, "Invalid offset 0x%08X of %s", start, *fileName);
		return ExpressionValue();
	}

	char buffer[1024];
	bool stringTerminated = false;
	std::string result;

	for (int64_t progress = 0; !stringTerminated && progress < length; progress += (int64_t) sizeof(buffer))
	{
		auto bytesToRead = (size_t) std::min((int64_t) sizeof(buffer), length - progress);

		file.read(buffer, bytesToRead);
		if (file.fail())
		{
			Logger::queueError(Logger::Error, "Failed to read %d byte(s) from offset 0x%08X of %s", bytesToRead, *fileName);
			return ExpressionValue();
		}

		for (std::streamsize i = 0; i < file.gcount(); i++)
		{
			if (buffer[i] == 0x00)
			{
				stringTerminated = true;
				break;
			}

			if (buffer[i] < 0x20)
			{
				Logger::printError(Logger::Warning, "%s: Non-ASCII character", funcName);
				return ExpressionValue();
			}

			result += buffer[i];
		}
	}

	return ExpressionValue(result);
}

ExpressionValue expLabelFuncDefined(const Identifier &funcName, const std::vector<std::shared_ptr<Label>> &parameters)
{
	if (parameters.empty() || !parameters.front())
	{
		Logger::queueError(Logger::Error, "%s: Invalid parameters", funcName);
		return ExpressionValue();
	}

	return ExpressionValue(parameters.front()->isDefined() ? INT64_C(1) : INT64_C(0));
}

ExpressionValue expLabelFuncOrg(const Identifier &funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return physical address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		return ExpressionValue(parameters.front()->getValue());
	}

	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error, "%s: no file opened", funcName);
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getVirtualAddress());
}

ExpressionValue expLabelFuncOrga(const Identifier &funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return physical address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		if (!label->hasPhysicalValue())
		{
			Logger::queueError(Logger::Error, "%s: parameter %s has no physical address", funcName, label->getName());
			return ExpressionValue();
		}

		return ExpressionValue(parameters.front()->getPhysicalValue());
	}

	// return current physical address otherwise
	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error, "%s: no file opened", funcName);
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getPhysicalAddress());
}

ExpressionValue expLabelFuncHeaderSize(const Identifier &funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return difference between physical and virtual address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		if (!label->hasPhysicalValue())
		{
			Logger::queueError(Logger::Error, "%s: parameter %s has no physical address", funcName, label->getName());
			return ExpressionValue();
		}

		return ExpressionValue(label->getValue() - label->getPhysicalValue());
	}

	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error, "headersize: no file opened");
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getHeaderSize());
}

const ExpressionFunctionEntry expressionFunctions[] = {
	{ "version",       &expFuncVersion,        0, 0,   ExpFuncSafety::Safe},
	{ "endianness",    &expFuncEndianness,     0, 0,   ExpFuncSafety::Unsafe},
	{ "outputname",    &expFuncOutputName,     0, 0,   ExpFuncSafety::Unsafe},
	{ "fileexists",    &expFuncFileExists,     1, 1,   ExpFuncSafety::Safe},
	{ "filesize",      &expFuncFileSize,       1, 1,   ExpFuncSafety::ConditionalUnsafe},
	{ "tostring",      &expFuncToString,       1, 1,   ExpFuncSafety::Safe},
	{ "tohex",         &expFuncToHex,          1, 2,   ExpFuncSafety::Safe},

	{ "int",           &expFuncInt,            1, 1,   ExpFuncSafety::Safe},
	{ "float",         &expFuncFloat,          1, 1,   ExpFuncSafety::Safe},
	{ "frac",          &expFuncFrac,           1, 1,   ExpFuncSafety::Safe},
	{ "abs",           &expFuncAbs,            1, 1,   ExpFuncSafety::Safe},
	{ "round",         &expFuncRound,          1, 1,   ExpFuncSafety::Safe},
	{ "min",           &expFuncMin,            1, 256, ExpFuncSafety::Safe},
	{ "max",           &expFuncMax,            1, 256, ExpFuncSafety::Safe},

	{ "strlen",        &expFuncStrlen,         1, 1,   ExpFuncSafety::Safe},
	{ "substr",        &expFuncSubstr,         3, 3,   ExpFuncSafety::Safe},
#if ARMIPS_REGEXP
	{ "regex_match",   &expFuncRegExMatch,     2, 2,   ExpFuncSafety::Safe},
	{ "regex_search",  &expFuncRegExSearch,    2, 2,   ExpFuncSafety::Safe},
	{ "regex_extract", &expFuncRegExExtract,   2, 3,   ExpFuncSafety::Safe},
#endif
	{ "find",          &expFuncFind,           2, 3,   ExpFuncSafety::Safe},
	{ "rfind",         &expFuncRFind,          2, 3,   ExpFuncSafety::Safe},

	{ "readbyte",      &expFuncRead<uint8_t>,  1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "readu8",        &expFuncRead<uint8_t>,  1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "readu16",       &expFuncRead<uint16_t>, 1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "readu32",       &expFuncRead<uint32_t>, 1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "readu64",       &expFuncRead<uint64_t>, 1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "reads8",        &expFuncRead<int8_t>,   1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "reads16",       &expFuncRead<int16_t>,  1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "reads32",       &expFuncRead<int32_t>,  1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "reads64",       &expFuncRead<int64_t>,  1, 2,   ExpFuncSafety::ConditionalUnsafe},
	{ "readascii",     &expFuncReadAscii,      1, 3,   ExpFuncSafety::ConditionalUnsafe},
};

const ExpressionLabelFunctionEntry expressionLabelFunctions[] =
{
	{ "defined",    &expLabelFuncDefined,      1, 1, ExpFuncSafety::Unsafe},
	{ "org",        &expLabelFuncOrg,          0, 1, ExpFuncSafety::Unsafe},
	{ "orga",       &expLabelFuncOrga,         0, 1, ExpFuncSafety::Unsafe},
	{ "headersize", &expLabelFuncHeaderSize,   0, 1, ExpFuncSafety::Unsafe},
};

void registerExpressionFunctions(ExpressionFunctionHandler &handler)
{
	for (const auto &func : expressionFunctions)
	{
		handler.addFunction(Identifier(func.name), func.function, func.minParams, func.maxParams, func.safety);
	}

	for (const auto &func : expressionLabelFunctions)
	{
		handler.addLabelFunction(Identifier(func.name), func.function, func.minParams, func.maxParams, func.safety);
	}
}
