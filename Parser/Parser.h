#pragma once
#include "Tokenizer.h"
#include "Core/Expression.h"
#include "Commands/CommandSequence.h"

CAssemblerCommand* parseCommand(Tokenizer& tokenizer);
bool parseExpressionList(Tokenizer& tokenizer, std::vector<Expression>& list);
bool parseIdentifier(Tokenizer& tokenizer, std::wstring& dest);
bool checkExpressionListSize(std::vector<Expression>& list, int min, int max);
CommandSequence* parseCommandSequence(Tokenizer& tokenizer, std::initializer_list<wchar_t*> terminators);
CommandSequence* parseFile(TextFile& file);
CommandSequence* parseString(const std::wstring& text);


