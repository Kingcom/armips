#include "Archs/ARM/ArmExpressionFunctions.h"

#include "Archs/ARM/Arm.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/ExpressionFunctions.h"

#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();

ExpressionValue expFuncIsArm(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	bool isArm = Arch == &Arm && !Arm.GetThumbMode();
	return ExpressionValue(isArm ? INT64_C(1) : INT64_C(0));
}

ExpressionValue expFuncIsThumb(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	bool isThumb = Arm.GetThumbMode();
	return ExpressionValue(isThumb ? INT64_C(1) : INT64_C(0));
}

const ExpressionFunctionMap armExpressionFunctions = {
	{ L"isarm",			{ &expFuncIsArm,			0,	0,	ExpFuncSafety::Safe } },
	{ L"isthumb",		{ &expFuncIsThumb,			0,	0,	ExpFuncSafety::Safe } },
};
