#pragma once
#include "Tokenizer.h"
#include "Core/Expression.h"

Expression parseExpression(Tokenizer& tokenizer);
void allowFunctionCallExpression(bool allow);
