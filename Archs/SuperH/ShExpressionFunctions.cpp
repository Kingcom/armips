#include "Archs/SuperH/ShExpressionFunctions.h"

#include "Core/Expression.h"
#include "Core/ExpressionFunctions.h"

#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();

/*
const ExpressionFunctionEntry shExpressionFunctions[] = {
};
*/

void registerShExpressionFunctions(ExpressionFunctionHandler &handler)
{
/*
	for (const auto &func : shExpressionFunctions)
	{
		handler.addFunction(Identifier(func.name), func.function, func.minParams, func.maxParams, func.safety);
	}
*/
}
