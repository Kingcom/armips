#pragma once

#include <memory>
#include <string>

class CAssemblerCommand;
class CArmInstruction;
class CThumbInstruction;
class Expression;
class Parser;

struct ArmRegisterValue;
struct ArmOpcodeVariables;
struct ThumbOpcodeVariables;
struct tArmOpcode;
struct tThumbOpcode;

struct ArmRegisterDescriptor {
	const char* name;
	int num;
};

class ArmParser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CArmInstruction> parseArmOpcode(Parser& parser);
	std::unique_ptr<CThumbInstruction> parseThumbOpcode(Parser& parser);
private:
	bool parseRegisterTable(Parser& parser, ArmRegisterValue& dest, const ArmRegisterDescriptor* table, size_t count);
	bool parseRegister(Parser& parser, ArmRegisterValue& dest, int max = 15);
	bool parseCopRegister(Parser& parser, ArmRegisterValue& dest);
	bool parseCopNumber(Parser& parser, ArmRegisterValue& dest);
	bool parseRegisterList(Parser& parser, int& dest, int validMask);
	bool parseImmediate(Parser& parser, Expression& dest);
	bool parseShift(Parser& parser, ArmOpcodeVariables& vars, bool immediateOnly);
	bool parsePseudoShift(Parser& parser, ArmOpcodeVariables& vars, int type);
	void parseWriteback(Parser& parser, bool& dest);
	void parsePsr(Parser& parser, bool& dest);
	void parseSign(Parser& parser, bool& dest);
	bool parsePsrTransfer(Parser& parser, ArmOpcodeVariables& vars, bool shortVersion);
	
	bool matchSymbol(Parser& parser, char symbol, bool optional);
	
	int decodeCondition(const std::string& text, size_t& pos);
	bool decodeAddressingMode(const std::string& text, size_t& pos, unsigned char& dest);
	bool decodeXY(const std::string& text, size_t& pos, bool& dest);
	void decodeS(const std::string& text, size_t& pos, bool& dest);
	bool decodeArmOpcode(const std::string& name, const tArmOpcode& opcode, ArmOpcodeVariables& vars);
	
	bool parseArmParameters(Parser& parser, const tArmOpcode& opcode, ArmOpcodeVariables& vars);
	bool parseThumbParameters(Parser& parser, const tThumbOpcode& opcode, ThumbOpcodeVariables& vars);
};
