#include "Archs/MIPS/MipsExpressionFunctions.h"

#include "Core/Expression.h"
#include "Core/ExpressionFunctions.h"

#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();

ExpressionValue expFuncHi(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value;

	GET_PARAM(parameters,0,value);

	return ExpressionValue((int64_t)((value >> 16) + ((value & 0x8000) != 0)) & 0xFFFF);
}

ExpressionValue expFuncLo(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value;

	GET_PARAM(parameters,0,value);

	return ExpressionValue((int64_t)(int16_t)(value & 0xFFFF));
}

const ExpressionFunctionEntry mipsExpressionFunctions[] = {
	{ L"lo",			&expFuncLo,				1,	1,	ExpFuncSafety::Safe },
	{ L"hi",			&expFuncHi,				1,	1,	ExpFuncSafety::Safe },
};

#include "Core/ExpressionFunctionHandler.h"

void registerMipsExpressionFunctions(ExpressionFunctionHandler &handler)
{
	for (const auto &func : mipsExpressionFunctions)
	{
		handler.addFunction(func.name, func.function, func.minParams, func.maxParams, func.safety);
	}
}
