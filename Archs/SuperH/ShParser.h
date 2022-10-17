#pragma once

#include "Archs/SuperH/CShInstruction.h"

#include <memory>
#include <string>

class CAssemblerCommand;
class Expression;
class Parser;
struct tShOpcode;

struct ShRegisterDescriptor {
	const char* name;
	int num;
};

class ShParser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CShInstruction> parseOpcode(Parser& parser);
private:
	bool parseRegisterTable(Parser& parser, ShRegisterValue& dest, const ShRegisterDescriptor* table, size_t count);
	bool parseRegister(Parser& parser, ShRegisterValue& dest);
	bool parseImmediate(Parser& parser, Expression& dest);

	bool decodeOpcode(Parser& parser, const tShOpcode& opcode);

	void setOmittedRegisters(const tShOpcode& opcode);
	bool matchSymbol(Parser& parser, char symbol);
	bool parseParameters(Parser& parser, const tShOpcode& opcode);

	ShRegisterData registers;
	ShImmediateData immediate;
	ShOpcodeData opcodeData;
};

class ShOpcodeFormatter
{
public:
	const std::string& formatOpcode(const ShOpcodeData& opData, const ShRegisterData& regData,
		const ShImmediateData& immData);
private:
	void handleOpcodeName(const ShOpcodeData& opData);
	void handleOpcodeParameters(const ShOpcodeData& opData, const ShRegisterData& regData,
		const ShImmediateData& immData);
	void handleImmediate(ShImmediateType type, unsigned int originalValue, unsigned int opcodeFlags);

	std::string buffer;
};
