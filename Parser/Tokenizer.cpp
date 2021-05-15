#include "Parser/Tokenizer.h"

#include "Core/Common.h"
#include "Util/Util.h"

#include <algorithm>

#include <tinyformat.h>

//
// Tokenizer
//

std::vector<std::vector<Token>> Tokenizer::equValues;

Tokenizer::Tokenizer()
{
	position.it = tokens.begin();
	invalidToken.type = TokenType::Invalid;
	invalidToken.setOriginalText(L"Unexpected end of token stream");
}

bool Tokenizer::processElement(TokenList::iterator& it)
{
	if (it == tokens.end())
		return false;

	while (!(*it).checked)
	{
		bool replaced = false;
		if ((*it).type == TokenType::Identifier)
		{
			const std::wstring stringValue = (*it).getStringValue();
			for (const Replacement& replacement: replacements)
			{
				// if the identifier matches, add all of its tokens
				if (replacement.identifier == stringValue)
				{
					TokenList::iterator insertIt = it;
					insertIt++;
				
					// replace old token with the new tokens
					// replace the first token manually so that any iterators
					// are still guaranteed to be valid
					(*it) = replacement.value[0];
					tokens.insert(insertIt,replacement.value.begin()+1, replacement.value.end());

					// If the value at this position didn't change, then just keep going.
					// Otherwise we'd be stuck in an endless replace loop
					if (stringValue != (*it).getStringValue())
						replaced = true;
					break;
				}
			}

			if (replaced)
				continue;

			// check for equs
			size_t index;
			if (Global.symbolTable.findEquation(stringValue,Global.FileInfo.FileNum,Global.Section,index))
			{
				TokenList::iterator nextIt = it;
				std::advance(nextIt, 1);
			
				// check if this is another equ with the same name.
				// if so, keep equ redefinitions for later error handling
				if (nextIt != tokens.end() && nextIt->type == TokenType::Equ)
					break;

				// make room for the replacement tokens
				const std::vector<Token>& replacement = equValues[index];
				tokens.insert(nextIt, replacement.size()-1, {});

				// insert replacement tokens, while keeping the file info of the original token
				Token originalToken = *it;

				TokenList::iterator insertIt = it;
				for (const Token& token: replacement)
				{
					(*insertIt) = token;
					insertIt->line = originalToken.line;
					insertIt->column = originalToken.column;
					std::advance(insertIt, 1);
				}

				replaced = true;
				continue;
			}
		}

		if (!replaced)
			(*it).checked = true;
	}

	return true;
}

const Token& Tokenizer::nextToken()
{
	if (!processElement(position.it))
		return invalidToken;

	return *position.it++;
}

const Token& Tokenizer::peekToken(int ahead)
{
	auto it = position.it;
	for (int i = 0; i < ahead; i++)
	{
		if (!processElement(it))
			return invalidToken;

		it++;
	}
	
	if (!processElement(it))
		return invalidToken;

	return *it;
}

void Tokenizer::eatTokens(int num)
{
	for (int i = 0; i < num; i++)
	{
		if (!processElement(position.it))
			break;
		position.it++;
	}
}

void Tokenizer::skipLookahead()
{
	//position.index = tokens.size();
}

std::vector<Token> Tokenizer::getTokens(TokenizerPosition start, TokenizerPosition end) const
{
	std::vector<Token> result;

	for (auto it = start.it; it != end.it; it++)
	{
		Token tok = *it;
		tok.checked = false;
		result.push_back(tok);
	}

	return result;
}

void Tokenizer::registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens)
{
	Replacement replacement { identifier, tokens };
	replacements.push_back(replacement);
}

void Tokenizer::registerReplacement(const std::wstring& identifier, const std::wstring& newValue)
{
	// Ensure the new identifier is lower case as it would be as a normally parsed string
	std::wstring lowerCase = newValue;
	std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::towlower);

	Token tok;
	tok.type = TokenType::Identifier;
	tok.setStringValue(lowerCase);
	tok.setOriginalText(newValue);

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::addToken(Token token)
{
	tokens.push_back(std::move(token));
}

size_t Tokenizer::addEquValue(const std::vector<Token>& tokens)
{
	size_t index = equValues.size();
	equValues.push_back(tokens);
	return index;
}

void Tokenizer::resetLookaheadCheckMarks()
{
	auto it = position.it;
	while (it != tokens.end() && it->checked)
	{
		it->checked = false;
		it++;
	}
}

//
// FileTokenizer
//

inline bool isWhitespace(const std::wstring& text, size_t pos)
{
	if (pos >= text.size())
		return false;

	return text[pos] == ' ' || text[pos] == '\t';
}

inline bool isComment(const std::wstring& text, size_t pos)
{
	if (pos < text.size() && text[pos] == ';')
		return true;

	if (pos+1 < text.size() && text[pos+0] == '/' && text[pos+1] == '/')
		return true;

	return false;
}

inline bool isContinuation(const std::wstring& text, size_t pos)
{
	if (pos >= text.size())
		return false;

	return text[pos] == '\\';
}

inline bool isBlockComment(const std::wstring& text, size_t pos){
	return pos+1 < text.size() && text[pos+0] == '/' && text[pos+1] == '*';
}

inline bool isBlockCommentEnd(const std::wstring& text, size_t pos){
	return pos+1 < text.size() && text[pos+0] == '*' && text[pos+1] == '/';
}

inline bool isValidIdentifierCharacter(wchar_t character, bool first)
{
	if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')) return true;
	if (!first && (character >= '0' && character <= '9')) return true;
	if (!first && character == '\'') return true;
	if (character == '_' || character == '.') return true;
	if (character == '@') return true;
	return false;
}

void FileTokenizer::skipWhitespace()
{
	while (true)
	{
		if (isWhitespace(currentLine,linePos))
		{
			do { linePos++; } while (isWhitespace(currentLine,linePos));
		} else if (isComment(currentLine,linePos))
		{
			linePos = currentLine.size();
		} else if (isBlockComment(currentLine,linePos))
		{
			linePos += 2;
			while(!isBlockCommentEnd(currentLine,linePos))
			{
				linePos++;
				if (linePos >= currentLine.size())
				{
					if (isInputAtEnd())
					{
						createToken(TokenType::Invalid,linePos,L"Unexpected end of file in block comment");
						addToken(token);
						return;
					}
					currentLine = input->readLine();
					linePos = 0;
					lineNumber++;
				}
			}
			linePos += 2;
		} else
		{
			break;
		}
	}
}

void FileTokenizer::createToken(TokenType type, size_t length)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, int64_t value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.intValue = value;

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, double value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.floatValue = value;

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, const std::wstring& value)
{
	createToken(type, length, value, 0, value.length());
}

void FileTokenizer::createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.setStringValue(value,valuePos,valueLength);

	linePos += length;
}

void FileTokenizer::createTokenCurrentString(TokenType type, size_t length)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setStringAndOriginalValue(currentLine,linePos,length);

	linePos += length;
}

bool FileTokenizer::parseOperator()
{
	wchar_t first = currentLine[linePos];
	wchar_t second = linePos+1 >= currentLine.size() ? '\0' : currentLine[linePos+1];

	switch (first)
	{
	case '(':
		createToken(TokenType::LParen,1);
		return true;
	case ')':
		createToken(TokenType::RParen,1);
		return true;
	case '+':
		createToken(TokenType::Plus,1);
		return true;
	case '-':
		createToken(TokenType::Minus,1);
		return true;
	case '*':
		createToken(TokenType::Mult,1);
		return true;
	case '/':
		createToken(TokenType::Div,1);
		return true;
	case '%':
		createToken(TokenType::Mod,1);
		return true;
	case '^':
		createToken(TokenType::Caret,1);
		return true;
	case '~':
		createToken(TokenType::Tilde,1);
		return true;
	case '<':
		if (second == '<')
			createToken(TokenType::LeftShift,2);
		else if (second == '=')
			createToken(TokenType::LessEqual,2);
		else
			createToken(TokenType::Less,1);
		return true;
	case '>':
		if (second == '>')
			createToken(TokenType::RightShift,2);
		else if (second == '=')
			createToken(TokenType::GreaterEqual,2);
		else
			createToken(TokenType::Greater,1);
		return true;
	case '=':
		if (second == '=')
			createToken(TokenType::Equal,2);
		else
			createToken(TokenType::Assign,1);
		return true;
	case '!':
		if (second == '=')
			createToken(TokenType::NotEqual,2);
		else
			createToken(TokenType::Exclamation,1);
		return true;
	case '&':
		if (second == '&')
			createToken(TokenType::LogAnd,2);
		else
			createToken(TokenType::BitAnd,1);
		return true;
	case '|':
		if (second == '|')
			createToken(TokenType::LogOr,2);
		else
			createToken(TokenType::BitOr,1);
		return true;
	case '?':
		createToken(TokenType::Question,1);
		return true;
	case ':':
		if (second == ':')
			createToken(TokenType::Separator,2);
		else
			createToken(TokenType::Colon,1);
		return true;
	case ',':
		createToken(TokenType::Comma,1);
		return true;
	case '[':
		createToken(TokenType::LBrack,1);
		return true;
	case ']':
		createToken(TokenType::RBrack,1);
		return true;
	case '#':
		createToken(TokenType::Hash,1);
		return true;
	case '{':
		createToken(TokenType::LBrace,1);
		return true;
	case '}':
		createToken(TokenType::RBrace,1);
		return true;
	case '$':
		createToken(TokenType::Dollar,1);
		return true;
	case L'\U000000B0':	// degree sign
		createToken(TokenType::Degree,1);
		return true;
	}

	return false;
}

bool FileTokenizer::convertInteger(size_t start, size_t end, int64_t& result)
{
	return stringToInt(currentLine, start, end, result);
}

bool FileTokenizer::convertFloat(size_t start, size_t end, double& result)
{
	std::wstring str = currentLine.substr(start, end - start);
	wchar_t* end_ptr;

	result = wcstod(str.c_str(), &end_ptr);
	return str.c_str() + str.size() == end_ptr;
}

Token FileTokenizer::loadToken()
{
	if (isInputAtEnd())
	{
		createToken(TokenType::Invalid,0);
		return std::move(token);
	}

	size_t pos = linePos;

	if (equActive)
	{
		while (pos < currentLine.size() && !isComment(currentLine,pos))
			pos++;

		createTokenCurrentString(TokenType::EquValue,pos-linePos);

		equActive = false;
		return std::move(token);
	}

	if (parseOperator())
		return std::move(token);

	wchar_t first = currentLine[pos];

	// character constants
	if (first == '\'' && pos+2 < currentLine.size() && currentLine[pos+2] == '\'')
	{
		createToken(TokenType::Integer,3,(int64_t)currentLine[pos+1]);
		return std::move(token);
	}

	// strings
	if (first == '"')
	{
		std::wstring text;
		pos++;

		bool valid = false;
		while (pos < currentLine.size())
		{
			if (pos+1 < currentLine.size() && currentLine[pos] == '\\')
			{
				if (currentLine[pos+1] == '"')
				{
					text += '"';
					pos += 2;
					continue;
				}
				
				if (currentLine[pos+1] == '\\')
				{
					text += '\\';
					pos += 2;
					continue;
				}
			}

			if (currentLine[pos] == '"')
			{
				pos++;
				valid = true;
				break;
			}

			text += currentLine[pos++];
		}

		if (!valid)
		{
			createToken(TokenType::Invalid,pos-linePos,L"Unexpected end of line in string constant");
			return std::move(token);
		}
		
		createToken(TokenType::String,pos-linePos,text);
		return std::move(token);
	}

	// numbers
	if (first >= '0' && first <= '9')
	{
		// find end of number
		size_t start = pos;
		size_t end = pos;
		bool isValid = true;
		bool foundPoint = false;
		bool foundExp = false;
		bool foundExpSign = false;
		bool isHex = start+1 < currentLine.size() && currentLine[start] == '0' && towlower(currentLine[start+1]) == 'x';

		while (end < currentLine.size() && (iswalnum(currentLine[end]) || currentLine[end] == '.'))
		{
			if (currentLine[end] == '.')
			{
				if (foundExp || foundPoint)
					isValid = false;
				foundPoint = true;
			} else if (towlower(currentLine[end]) == 'h' && !foundExpSign) {
				isHex = true;
			} else if (towlower(currentLine[end]) == 'e' && !isHex)
			{
				if (foundExp)
				{
					isValid = false;
				} else if (end+1 < currentLine.size() && (currentLine[end+1] == '+' || currentLine[end+1] == '-')){
					end++;
					if (end+1 >= currentLine.size() || !iswalnum(currentLine[end+1]))
						isValid = false;
					foundExpSign = true;
				}
				foundExp = true;
			}

			end++;
		}

		bool isFloat = foundPoint || (foundExp && !isHex);

		if (!isFloat)
		{
			int64_t value;
			if (!convertInteger(start,end,value))
			{
				createTokenCurrentString(TokenType::NumberString,end-start);
				return std::move(token);
			}

			createToken(TokenType::Integer,end-start,value);
		} else { // isFloat
			double value;
			if (!isValid)
			{
				createToken(TokenType::Invalid,end-start,L"Invalid floating point number");
				return std::move(token);
			}

			if (!convertFloat(start,end,value))
			{
				createTokenCurrentString(TokenType::NumberString,end-start);
				return std::move(token);
			}

			createToken(TokenType::Float,end-start,value);
		}
		
		return std::move(token);
	}

	// identifiers
	bool isFirst = true;
	while (pos < currentLine.size() && isValidIdentifierCharacter(currentLine[pos],isFirst))
	{
		pos++;
		isFirst = false;
	}

	if (pos == linePos)
	{
		std::wstring text = tfm::format(L"Invalid input '%c'",currentLine[pos]);
		createToken(TokenType::Invalid,1,text);
		return std::move(token);
	}

	std::wstring text = currentLine.substr(linePos,pos-linePos);
	bool textLowered = false;
	// Lowercase is common, let's try to avoid a copy.
	if (std::any_of(text.begin(), text.end(), ::iswupper))
	{
		std::transform(text.begin(), text.end(), text.begin(), ::towlower);
		textLowered = true;
	}

	if (text == L"equ")
	{
		createToken(TokenType::Equ,pos-linePos);
		equActive = true;
	} else if (textLowered) {
		createToken(TokenType::Identifier,pos-linePos,text);
	} else {
		createTokenCurrentString(TokenType::Identifier,pos-linePos);
	}

	return std::move(token);
}

bool FileTokenizer::isInputAtEnd()
{
	return linePos >= currentLine.size() && input->atEnd();
}

bool FileTokenizer::init(TextFile* input)
{
	clearTokens();

	lineNumber = 1;
	linePos = 0;
	equActive = false;
	currentLine = input->readLine();

	this->input = input;
	if (input != nullptr && input->isOpen())
	{
		while (!isInputAtEnd())
		{
			bool addSeparator = true;

			skipWhitespace();
			if (isContinuation(currentLine, linePos))
			{
				linePos++;
				skipWhitespace();
				if (linePos < currentLine.size())
				{
					createToken(TokenType::Invalid,0,
						L"Unexpected character after line continuation character");
					addToken(token);
				}

				addSeparator = false;
			} else if(linePos < currentLine.size())
			{
				addToken(loadToken());
			}

			if (linePos >= currentLine.size())
			{
				if (addSeparator)
				{
					createToken(TokenType::Separator,0);
					addToken(token);
				}

				if (input->atEnd())
					break;

				currentLine = input->readLine();
				linePos = 0;
				lineNumber++;
			}
		}

		resetPosition();
		return true;
	}

	return false;
}
