#pragma once
#include "Parser/Tokenizer.h"
#include "CThumbInstruction.h"
#include "CArmInstruction.h"

struct ArmRegisterDescriptor {
	const wchar_t* name;
	int num;
};

class ArmParser
{
public:
	CAssemblerCommand* parseDirective(Tokenizer& tokenizer);
	CArmInstruction* parseArmOpcode(Tokenizer& tokenizer);
	CThumbInstruction* parseThumbOpcode(Tokenizer& tokenizer);
private:
	bool parseRegisterTable(Tokenizer& tokenizer, ArmRegisterValue& dest, const ArmRegisterDescriptor* table, size_t count);
	bool parseRegister(Tokenizer& tokenizer, ArmRegisterValue& dest, int max = 15);
	bool parseCopRegister(Tokenizer& tokenizer, ArmRegisterValue& dest);
	bool parseCopNumber(Tokenizer& tokenizer, ArmRegisterValue& dest);
	bool parseRegisterList(Tokenizer& tokenizer, int& dest, int validMask);
	bool parseImmediate(Tokenizer& tokenizer, Expression& dest);
	bool parseShift(Tokenizer& tokenizer, ArmOpcodeVariables& vars, bool immediateOnly);
	bool parsePseudoShift(Tokenizer& tokenizer, ArmOpcodeVariables& vars, int type);
	void parseWriteback(Tokenizer& tokenizer, bool& dest);
	void parsePsr(Tokenizer& tokenizer, bool& dest);
	void parseSign(Tokenizer& tokenizer, bool& dest);
	bool parsePsrTransfer(Tokenizer& tokenizer, ArmOpcodeVariables& vars, bool shortVersion);
	
	bool matchSymbol(Tokenizer& tokenizer, wchar_t symbol, bool optional);
	
	int decodeCondition(const std::wstring& text, size_t& pos);
	bool decodeAddressingMode(const std::wstring& text, size_t& pos, unsigned char& dest);
	bool decodeXY(const std::wstring& text, size_t& pos, bool& dest);
	void decodeS(const std::wstring& text, size_t& pos, bool& dest);
	bool decodeArmOpcode(const std::wstring& name, const tArmOpcode& opcode, ArmOpcodeVariables& vars);
	
	bool parseArmParameters(Tokenizer& tokenizer, const tArmOpcode& opcode, ArmOpcodeVariables& vars);
	bool parseThumbParameters(Tokenizer& tokenizer, const tThumbOpcode& opcode, ThumbOpcodeVariables& vars);
};