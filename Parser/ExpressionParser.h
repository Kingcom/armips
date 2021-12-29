#pragma once

class Expression;
class Tokenizer;

Expression parseExpression(Tokenizer& tokenizer, bool inUnknownOrFalseBlock);
