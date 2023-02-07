#include "Parser/DirectivesParser.h"

#include "Archs/ARM/Arm.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/SuperH/SuperH.h"
#include "Commands/CAssemblerLabel.h"
#include "Commands/CDirectiveArea.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CommandSequence.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/Misc.h"
#include "Parser/ExpressionParser.h"
#include "Parser/Parser.h"
#include "Parser/Tokenizer.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include <algorithm>
#include <initializer_list>

std::unique_ptr<CAssemblerCommand> parseDirectiveOpen(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,2,3))
		return nullptr;

	int64_t memoryAddress;
	StringLiteral inputName, outputName;

	if (!list[0].evaluateString(inputName,false))
		return nullptr;

	if (!list.back().evaluateInteger(memoryAddress))
		return nullptr;

	auto file = std::make_unique<CDirectiveFile>();
	if (list.size() == 3)
	{
		if (!list[1].evaluateString(outputName,false))
			return nullptr;
		
		file->initCopy(inputName.path(),outputName.path(),memoryAddress);
		return file;
	} else {
		file->initOpen(inputName.path(),memoryAddress);
		return file;
	}
}

std::unique_ptr<CAssemblerCommand> parseDirectiveCreate(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,2,2))
		return nullptr;

	int64_t memoryAddress;
	StringLiteral inputName;

	if (!list[0].evaluateString(inputName,false))
		return nullptr;

	if (!list.back().evaluateInteger(memoryAddress))
		return nullptr;

	auto file = std::make_unique<CDirectiveFile>();
	file->initCreate(inputName.path(),memoryAddress);
	return file;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveClose(Parser& parser, int flags)
{
	auto file = std::make_unique<CDirectiveFile>();
	file->initClose();
	return file;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveIncbin(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,3))
		return nullptr;
	
	StringLiteral fileName;
	if (!list[0].evaluateString(fileName,false))
		return nullptr;

	auto incbin = std::make_unique<CDirectiveIncbin>(fileName.path());
	if (list.size() >= 2)
		incbin->setStart(list[1]);

	if (list.size() == 3)
		incbin->setSize(list[2]);

	return incbin;
}

std::unique_ptr<CAssemblerCommand> parseDirectivePosition(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();
	if (!exp.isLoaded())
		return nullptr;

	CDirectivePosition::Type type;
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_POS_PHYSICAL:
		type = CDirectivePosition::Physical;
		break;
	case DIRECTIVE_POS_VIRTUAL:
		type = CDirectivePosition::Virtual;
		break;
	default:
		return nullptr;
	}

	return std::make_unique<CDirectivePosition>(exp,type);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveAlignFill(Parser& parser, int flags)
{
	CDirectiveAlignFill::Mode mode;
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_ALIGN_VIRTUAL:
		mode = CDirectiveAlignFill::AlignVirtual;
		break;
	case DIRECTIVE_ALIGN_PHYSICAL:
		mode = CDirectiveAlignFill::AlignPhysical;
		break;
	case DIRECTIVE_ALIGN_FILL:
		mode = CDirectiveAlignFill::Fill;
		break;
	default:
		return nullptr;
	}

	if (mode != CDirectiveAlignFill::Fill && parser.peekToken().type == TokenType::Separator)
		return std::make_unique<CDirectiveAlignFill>(UINT64_C(4),mode);

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	if (list.size() == 2)
		return std::make_unique<CDirectiveAlignFill>(list[0],list[1],mode);
	else
		return std::make_unique<CDirectiveAlignFill>(list[0],mode);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveSkip(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,1))
		return nullptr;

	return std::make_unique<CDirectiveSkip>(list[0]);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveHeaderSize(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();
	if (!exp.isLoaded())
		return nullptr;

	return std::make_unique<CDirectiveHeaderSize>(exp);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveObjImport(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	StringLiteral fileName;
	if (!list[0].evaluateString(fileName,true))
		return nullptr;

	if (list.size() == 2)
	{
		Identifier ctorName;
		if (!list[1].evaluateIdentifier(ctorName))
			return nullptr;

		return std::make_unique<DirectiveObjImport>(fileName.path(),ctorName);
	}
	
	return std::make_unique<DirectiveObjImport>(fileName.path());
}

std::unique_ptr<CAssemblerCommand> parseDirectiveConditional(Parser& parser, int flags)
{
	ConditionType type;
	Identifier name;
	Expression exp;

	const Token& start = parser.peekToken();
	ConditionalResult condResult = ConditionalResult::Unknown;
	switch (flags)
	{
	case DIRECTIVE_COND_IF:
		type = ConditionType::IF;
		exp = parser.parseExpression();
		if (!exp.isLoaded())
		{
			parser.printError(start, "Invalid condition");
			return std::make_unique<DummyCommand>();
		}

		if (exp.isConstExpression())
		{
			ExpressionValue result = exp.evaluate();
			if (result.isInt())
				condResult = result.intValue != 0 ? ConditionalResult::True : ConditionalResult::False;
		}
		break;
	case DIRECTIVE_COND_IFDEF:
		type = ConditionType::IFDEF;
		if (!parser.parseIdentifier(name))
			return nullptr;		
		break;
	case DIRECTIVE_COND_IFNDEF:
		type = ConditionType::IFNDEF;
		if (!parser.parseIdentifier(name))
			return nullptr;
		break;
	}

	if(parser.nextToken().type != TokenType::Separator)
	{
		parser.printError(start, "Directive not terminated");
		return nullptr;
	}

	parser.pushConditionalResult(condResult);
	std::unique_ptr<CAssemblerCommand> ifBlock = parser.parseCommandSequence('.', {".else", ".elseif", ".elseifdef", ".elseifndef", ".endif"});
	parser.popConditionalResult();

	// update the file info so that else commands get the right line number
	parser.updateFileInfo();

	std::unique_ptr<CAssemblerCommand> elseBlock = nullptr;
	const Token &next = parser.nextToken();

	if (next.type != TokenType::Identifier)
	{
		parser.printError(start, "Expected identifier after if block");
		return nullptr;
	}

	const Identifier &identifier = next.identifierValue();

	ConditionalResult elseResult;
	switch (condResult)
	{
	case ConditionalResult::True:
		elseResult = ConditionalResult::False;
		break;
	case ConditionalResult::False:
		elseResult = ConditionalResult::True;
		break;
	case ConditionalResult::Unknown:
		elseResult = condResult;
		break;
	}

	parser.pushConditionalResult(elseResult);
	if (identifier == ".else")
	{
		elseBlock = parser.parseCommandSequence('.', {".endif"});

		parser.eatToken();	// eat .endif
	} else if (identifier == ".elseif")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IF);
	} else if (identifier == ".elseifdef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFDEF);
	} else if (identifier == ".elseifndef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFNDEF);
	} else if (identifier != ".endif")
	{
		parser.popConditionalResult();
		return nullptr;
	}

	parser.popConditionalResult();

	// for true or false blocks, there's no need to create a conditional command
	if (condResult == ConditionalResult::True)
	{
		return ifBlock;
	}
	
	if (condResult == ConditionalResult::False)
	{
		if (elseBlock != nullptr)
			return elseBlock;
		else
			return std::make_unique<DummyCommand>();
	}

	std::unique_ptr<CDirectiveConditional> cond;
	if (exp.isLoaded())
		cond = std::make_unique<CDirectiveConditional>(type,exp);
	else if (name.size() != 0)
		cond = std::make_unique<CDirectiveConditional>(type, name);
	else
		cond = std::make_unique<CDirectiveConditional>(type);

	cond->setContent(std::move(ifBlock),std::move(elseBlock));
	return cond;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveTable(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	StringLiteral fileName;
	if (!list[0].evaluateString(fileName,true))
	{
		parser.printError(start, "Invalid file name");
		return nullptr;
	}

	TextFile::Encoding encoding = TextFile::GUESS;
	if (list.size() == 2)
	{
		StringLiteral encodingName;
		if (!list[1].evaluateString(encodingName,true))
		{
			parser.printError(start, "Invalid encoding name");
			return nullptr;
		}

		encoding = getEncodingFromString(encodingName.string());
	}

	return std::make_unique<TableCommand>(fileName.path(),encoding);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveData(Parser& parser, int flags)
{
	bool terminate = false;
	if (flags & DIRECTIVE_DATA_TERMINATION)
	{
		terminate = true;
		flags &= ~DIRECTIVE_DATA_TERMINATION;
	}

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,-1))
		return nullptr;
	
	auto data = std::make_unique<CDirectiveData>();
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_DATA_8:
		data->setNormal(list,1);
		break;
	case DIRECTIVE_DATA_16:
		data->setNormal(list,2);
		break;
	case DIRECTIVE_DATA_32:
		data->setNormal(list,4);
		break;
	case DIRECTIVE_DATA_64:
		data->setNormal(list,8);
		break;
	case DIRECTIVE_DATA_HWORD:
		data->setNormal(list,Architecture::current().getWordSize()/2);
		break;
	case DIRECTIVE_DATA_WORD:
		data->setNormal(list,Architecture::current().getWordSize());
		break;
	case DIRECTIVE_DATA_DWORD:
		data->setNormal(list,Architecture::current().getWordSize()*2);
		break;
	case DIRECTIVE_DATA_ASCII:
		data->setAscii(list,terminate);
		break;
	case DIRECTIVE_DATA_SJIS:
		data->setSjis(list,terminate);
		break;
	case DIRECTIVE_DATA_CUSTOM:
		data->setCustom(list,terminate);
		break;
	case DIRECTIVE_DATA_FLOAT:
		data->setFloat(list);
		break;
	case DIRECTIVE_DATA_DOUBLE:
		data->setDouble(list);
		break;
	}
	
	return data;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveMipsArch(Parser& parser, int flags)
{
	Architecture::setCurrent(Mips);
	Mips.SetLoadDelay(false, 0);

	switch (flags)
	{
	case DIRECTIVE_MIPS_PSX:
		Mips.SetVersion(MARCH_PSX);
		return std::make_unique<ArchitectureCommand>(".psx", "");
	case DIRECTIVE_MIPS_PS2:
		Mips.SetVersion(MARCH_PS2);
		return std::make_unique<ArchitectureCommand>(".ps2", "");
	case DIRECTIVE_MIPS_PSP:
		Mips.SetVersion(MARCH_PSP);
		return std::make_unique<ArchitectureCommand>(".psp", "");
	case DIRECTIVE_MIPS_N64:
		Mips.SetVersion(MARCH_N64);
		return std::make_unique<ArchitectureCommand>(".n64", "");
	case DIRECTIVE_MIPS_RSP:
		Mips.SetVersion(MARCH_RSP);
		return std::make_unique<ArchitectureCommand>(".rsp", "");
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveArmArch(Parser& parser, int flags)
{
	Architecture::setCurrent(Arm);

	switch (flags)
	{
	case DIRECTIVE_ARM_GBA:
		Arm.SetThumbMode(true);
		Arm.setVersion(AARCH_GBA);
		return std::make_unique<ArchitectureCommand>(".gba\n.thumb", ".thumb");
	case DIRECTIVE_ARM_NDS:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_NDS);
		return std::make_unique<ArchitectureCommand>(".nds\n.arm", ".arm");
	case DIRECTIVE_ARM_3DS:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_3DS);
		return std::make_unique<ArchitectureCommand>(".3ds\n.arm", ".arm");
	case DIRECTIVE_ARM_BIG:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_BIG);
		return std::make_unique<ArchitectureCommand>(".arm.big\n.arm", ".arm");
	case DIRECTIVE_ARM_LITTLE:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_LITTLE);
		return std::make_unique<ArchitectureCommand>(".arm.little\n.arm", ".arm");
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveShArch(Parser& parser, int flags)
{
	Architecture::setCurrent(SuperH);

	switch (flags)
	{
	case DIRECTIVE_SH_SATURN:
		SuperH.setVersion(SHARCH_SATURN);
		return std::make_unique<ArchitectureCommand>(".saturn", "");
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveArea(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,1,2))
		return nullptr;

	bool shared = (flags & DIRECTIVE_AREA_SHARED) != 0;
	auto area = std::make_unique<CDirectiveArea>(shared, parameters[0]);
	if (parameters.size() == 2)
		area->setFillExpression(parameters[1]);

	std::unique_ptr<CAssemblerCommand> content = parser.parseCommandSequence('.', { ".endarea", ".endregion" });
	parser.eatToken();

	area->setContent(std::move(content));
	return area;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveDefineArea(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,2,3))
		return nullptr;

	bool shared = (flags & DIRECTIVE_AREA_SHARED) != 0;
	auto area = std::make_unique<CDirectiveArea>(shared, parameters[1]);
	area->setPositionExpression(parameters[0]);
	if (parameters.size() == 3)
		area->setFillExpression(parameters[2]);

	return area;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveAutoRegion(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (parser.peekToken().type != TokenType::Separator)
	{
		if (!parser.parseExpressionList(parameters, 0, 2))
			return nullptr;
	}

	auto area = std::make_unique<CDirectiveAutoRegion>();
	if (parameters.size() == 1)
		area->setMinRangeExpression(parameters[0]);
	else if (parameters.size() == 2)
		area->setRangeExpressions(parameters[0], parameters[1]);

	std::unique_ptr<CAssemblerCommand> content = parser.parseCommandSequence('.', {".endautoregion"});
	parser.eatToken();

	area->setContent(std::move(content));
	return area;
}

std::optional<std::string> getStringOrIdentifier(Parser& parser)
{
	const Token &tok = parser.nextToken();
	if (tok.type == TokenType::Identifier)
		return tok.identifierValue().string();

	if (tok.type == TokenType::String)
	{
		auto stringValue = tok.stringValue().string();
		std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::tolower);
		return stringValue;
	}

	return std::nullopt;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveErrorWarning(Parser& parser, int flags)
{
	auto stringValue = getStringOrIdentifier(parser);
	if (!stringValue)
		return nullptr;

	if (stringValue == "on")
	{	
		Logger::setErrorOnWarning(true);
		return std::make_unique<DummyCommand>();
	} else if (stringValue == "off")
	{
		Logger::setErrorOnWarning(false);
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveRelativeInclude(Parser& parser, int flags)
{
	auto stringValue = getStringOrIdentifier(parser);
	if (!stringValue)
		return nullptr;

	if (stringValue == "on")
	{	
		Global.relativeInclude = true;
		return std::make_unique<DummyCommand>();
	} else if (stringValue == "off")
	{
		Global.relativeInclude = false;
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveNocash(Parser& parser, int flags)
{
	auto stringValue = getStringOrIdentifier(parser);
	if (!stringValue)
		return nullptr;

	if (stringValue == "on")
	{	
		Global.nocash = true;
		return std::make_unique<DummyCommand>();
	} else if (stringValue == "off")
	{
		Global.nocash = false;
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveSym(Parser& parser, int flags)
{
	auto stringValue = getStringOrIdentifier(parser);
	if (!stringValue)
		return nullptr;

	if (stringValue == "on")
		return std::make_unique<CDirectiveSym>(true);
	else if (stringValue == "off")
		return std::make_unique<CDirectiveSym>(false);
	else
		return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveDefineLabel(Parser& parser, int flags)
{
	const Token& tok = parser.nextToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	if (parser.nextToken().type != TokenType::Comma)
		return nullptr;

	Expression value = parser.parseExpression();
	if (!value.isLoaded())
		return nullptr;

	const Identifier &identifier = tok.identifierValue();
	if (!Global.symbolTable.isValidSymbolName(identifier))
	{
		parser.printError(tok, "Invalid label name \"%s\"",identifier);
		return nullptr;
	}

	return std::make_unique<CAssemblerLabel>(identifier,Identifier(tok.getOriginalText()),value);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveFunction(Parser& parser, int flags)
{
	const Token& tok = parser.nextToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	if (parser.nextToken().type != TokenType::Separator)
	{
		parser.printError(tok, "Directive not terminated");
		return nullptr;
	}

	auto func = std::make_unique<CDirectiveFunction>(Identifier(tok.identifierValue()), Identifier(tok.getOriginalText()));
	std::unique_ptr<CAssemblerCommand> seq = parser.parseCommandSequence('.', {".endfunc",".endfunction",".func",".function"});

	const Token &next = parser.peekToken();
	if (next.type == TokenType::Identifier)
	{
		const Identifier &identifier = next.identifierValue();
		if (identifier == ".endfunc" || identifier == ".endfunction")
		{
			parser.eatToken();
			if(parser.nextToken().type != TokenType::Separator)
			{
				parser.printError(tok, "Directive not terminated");
				return nullptr;
			}
		}
	}

	func->setContent(std::move(seq));
	return func;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveMessage(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();

	switch (flags)
	{
	case DIRECTIVE_MSG_WARNING:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Warning,exp);
	case DIRECTIVE_MSG_ERROR:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Error,exp);
	case DIRECTIVE_MSG_NOTICE:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Notice,exp);
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveInclude(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,1,2))
		return nullptr;

	StringLiteral fileNameParameter;
	if (!parameters[0].evaluateString(fileNameParameter,true))
		return nullptr;

	auto fileName = getFullPathName(fileNameParameter.path());

	TextFile::Encoding encoding = TextFile::GUESS;
	if (parameters.size() == 2)
	{
		StringLiteral encodingName;
		if (!parameters[1].evaluateString(encodingName,true))
			return nullptr;
		
		encoding = getEncodingFromString(encodingName.string());
	}

	// don't include the file if it's inside a false block
	if (!parser.isInsideTrueBlock())
		return std::make_unique<DummyCommand>();

	if (!fs::exists(fileName))
	{
		parser.printError(start, "Included file \"%s\" does not exist",fileName.u8string());
		return nullptr;
	}

	TextFile f;
	if (!f.open(fileName,TextFile::Read,encoding))
	{
		parser.printError(start, "Could not open included file \"%s\"",fileName.u8string());
		return nullptr;
	}

	return parser.parseFile(f);
}

const DirectiveMap directives = {
	{ ".open",            { &parseDirectiveOpen,            DIRECTIVE_NOTINMEMORY } },
	{ ".openfile",        { &parseDirectiveOpen,            DIRECTIVE_NOTINMEMORY } },
	{ ".create",          { &parseDirectiveCreate,          DIRECTIVE_NOTINMEMORY } },
	{ ".createfile",      { &parseDirectiveCreate,          DIRECTIVE_NOTINMEMORY } },
	{ ".close",           { &parseDirectiveClose,           DIRECTIVE_NOTINMEMORY } },
	{ ".closefile",       { &parseDirectiveClose,           DIRECTIVE_NOTINMEMORY } },
	{ ".incbin",          { &parseDirectiveIncbin,          0 } },
	{ ".import",          { &parseDirectiveIncbin,          0 } },
	{ ".org",             { &parseDirectivePosition,        DIRECTIVE_POS_VIRTUAL } },
	{ "org",              { &parseDirectivePosition,        DIRECTIVE_POS_VIRTUAL } },
	{ ".orga",            { &parseDirectivePosition,        DIRECTIVE_POS_PHYSICAL } },
	{ "orga",             { &parseDirectivePosition,        DIRECTIVE_POS_PHYSICAL } },
	{ ".headersize",      { &parseDirectiveHeaderSize,      0 } },
	{ ".align",           { &parseDirectiveAlignFill,       DIRECTIVE_ALIGN_VIRTUAL } },
	{ ".aligna",          { &parseDirectiveAlignFill,       DIRECTIVE_ALIGN_PHYSICAL } },
	{ ".fill",            { &parseDirectiveAlignFill,       DIRECTIVE_ALIGN_FILL } },
	{ "defs",             { &parseDirectiveAlignFill,       DIRECTIVE_ALIGN_FILL } },
	{ ".skip",            { &parseDirectiveSkip,            0 } },

	{ ".if",              { &parseDirectiveConditional,     DIRECTIVE_COND_IF } },
	{ ".ifdef",           { &parseDirectiveConditional,     DIRECTIVE_COND_IFDEF } },
	{ ".ifndef",          { &parseDirectiveConditional,     DIRECTIVE_COND_IFNDEF } },

	{ ".loadtable",       { &parseDirectiveTable,           0 } },
	{ ".table",           { &parseDirectiveTable,           0 } },
	{ ".byte",            { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ ".halfword",        { &parseDirectiveData,            DIRECTIVE_DATA_HWORD } },
	{ ".hword",           { &parseDirectiveData,            DIRECTIVE_DATA_HWORD } },
	{ ".word",            { &parseDirectiveData,            DIRECTIVE_DATA_WORD } },
	{ ".doubleword",      { &parseDirectiveData,            DIRECTIVE_DATA_DWORD } },
	{ ".dword",           { &parseDirectiveData,            DIRECTIVE_DATA_DWORD } },
	{ ".db",              { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ ".dh",              { &parseDirectiveData,            DIRECTIVE_DATA_HWORD|DIRECTIVE_NOCASHOFF } },
	{ ".dw",              { &parseDirectiveData,            DIRECTIVE_DATA_WORD|DIRECTIVE_NOCASHOFF } },
	{ ".dd",              { &parseDirectiveData,            DIRECTIVE_DATA_DWORD|DIRECTIVE_NOCASHOFF } },
	{ ".dw",              { &parseDirectiveData,            DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ ".dd",              { &parseDirectiveData,            DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ ".dcb",             { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ ".dcw",             { &parseDirectiveData,            DIRECTIVE_DATA_HWORD } },
	{ ".dcd",             { &parseDirectiveData,            DIRECTIVE_DATA_WORD } },
	{ ".dcq",             { &parseDirectiveData,            DIRECTIVE_DATA_DWORD } },
	{ "db",               { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ "dh",               { &parseDirectiveData,            DIRECTIVE_DATA_HWORD|DIRECTIVE_NOCASHOFF } },
	{ "dw",               { &parseDirectiveData,            DIRECTIVE_DATA_WORD|DIRECTIVE_NOCASHOFF } },
	{ "dd",               { &parseDirectiveData,            DIRECTIVE_DATA_DWORD|DIRECTIVE_NOCASHOFF } },
	{ "dw",               { &parseDirectiveData,            DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ "dd",               { &parseDirectiveData,            DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ "dcb",              { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ "dcw",              { &parseDirectiveData,            DIRECTIVE_DATA_HWORD } },
	{ "dcd",              { &parseDirectiveData,            DIRECTIVE_DATA_WORD } },
	{ "dcq",              { &parseDirectiveData,            DIRECTIVE_DATA_DWORD } },
	{ ".d8",              { &parseDirectiveData,            DIRECTIVE_DATA_8 } },
	{ ".d16",             { &parseDirectiveData,            DIRECTIVE_DATA_16 } },
	{ ".d32",             { &parseDirectiveData,            DIRECTIVE_DATA_32 } },
	{ ".d64",             { &parseDirectiveData,            DIRECTIVE_DATA_64 } },
	{ ".float",           { &parseDirectiveData,            DIRECTIVE_DATA_FLOAT } },
	{ ".double",          { &parseDirectiveData,            DIRECTIVE_DATA_DOUBLE } },
	{ ".ascii",           { &parseDirectiveData,            DIRECTIVE_DATA_ASCII } },
	{ ".asciiz",          { &parseDirectiveData,            DIRECTIVE_DATA_ASCII|DIRECTIVE_DATA_TERMINATION } },
	{ ".string",          { &parseDirectiveData,            DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ ".str",             { &parseDirectiveData,            DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ ".stringn",         { &parseDirectiveData,            DIRECTIVE_DATA_CUSTOM } },
	{ ".strn",            { &parseDirectiveData,            DIRECTIVE_DATA_CUSTOM } },
	{ ".sjis",            { &parseDirectiveData,            DIRECTIVE_DATA_SJIS|DIRECTIVE_DATA_TERMINATION } },
	{ ".sjisn",           { &parseDirectiveData,            DIRECTIVE_DATA_SJIS } },

	{ ".psx",             { &parseDirectiveMipsArch,        DIRECTIVE_MIPS_PSX } },
	{ ".ps2",             { &parseDirectiveMipsArch,        DIRECTIVE_MIPS_PS2 } },
	{ ".psp",             { &parseDirectiveMipsArch,        DIRECTIVE_MIPS_PSP } },
	{ ".n64",             { &parseDirectiveMipsArch,        DIRECTIVE_MIPS_N64 } },
	{ ".rsp",             { &parseDirectiveMipsArch,        DIRECTIVE_MIPS_RSP } },

	{ ".gba",             { &parseDirectiveArmArch,         DIRECTIVE_ARM_GBA } },
	{ ".nds",             { &parseDirectiveArmArch,         DIRECTIVE_ARM_NDS } },
	{ ".3ds",             { &parseDirectiveArmArch,         DIRECTIVE_ARM_3DS } },
	{ ".arm.big",         { &parseDirectiveArmArch,         DIRECTIVE_ARM_BIG } },
	{ ".arm.little",      { &parseDirectiveArmArch,         DIRECTIVE_ARM_LITTLE } },

	{ ".saturn",          { &parseDirectiveShArch,          DIRECTIVE_SH_SATURN } },
	{ ".32x",             { &parseDirectiveShArch,          DIRECTIVE_SH_SATURN } },

	{ ".area",            { &parseDirectiveArea,            0 } },
	{ ".autoregion",      { &parseDirectiveAutoRegion,      0 } },
	{ ".region",          { &parseDirectiveArea,            DIRECTIVE_AREA_SHARED } },
	{ ".defineregion",    { &parseDirectiveDefineArea,      DIRECTIVE_AREA_SHARED } },

	{ ".importobj",       { &parseDirectiveObjImport,       0 } },
	{ ".importlib",       { &parseDirectiveObjImport,       0 } },

	{ ".erroronwarning",  { &parseDirectiveErrorWarning,    0 } },
	{ ".relativeinclude", { &parseDirectiveRelativeInclude, 0 } },
	{ ".nocash",          { &parseDirectiveNocash,          0 } },
	{ ".sym",             { &parseDirectiveSym,             0 } },

	{ ".definelabel",     { &parseDirectiveDefineLabel,     0 } },
	{ ".function",        { &parseDirectiveFunction,        DIRECTIVE_MANUALSEPARATOR } },
	{ ".func",            { &parseDirectiveFunction,        DIRECTIVE_MANUALSEPARATOR } },

	{ ".warning",         { &parseDirectiveMessage,         DIRECTIVE_MSG_WARNING } },
	{ ".error",           { &parseDirectiveMessage,         DIRECTIVE_MSG_ERROR } },
	{ ".notice",          { &parseDirectiveMessage,         DIRECTIVE_MSG_NOTICE } },

	{ ".include",         { &parseDirectiveInclude,         0 } },

};
