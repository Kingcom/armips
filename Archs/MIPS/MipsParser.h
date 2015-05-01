#pragma once
#include "Parser/Tokenizer.h"
#include "CMipsInstruction.h"

struct MipsRegisterDescriptor {
	const wchar_t* name;
	int num;
};

class MipsParser
{
public:
	CAssemblerCommand* parseDirective(Tokenizer& tokenizer);
	CMipsInstruction* parseOpcode(Tokenizer& tokenizer);
private:
	bool parseRegisterTable(Tokenizer& tokenizer, MipsRegisterValue& dest, const MipsRegisterDescriptor* table, size_t count);
	bool parseRegister(Tokenizer& tokenizer, MipsRegisterValue& dest);
	bool parseFpuRegister(Tokenizer& tokenizer, MipsRegisterValue& dest);
	bool parsePs2Cop2Register(Tokenizer& tokenizer, MipsRegisterValue& dest);
	bool parseVfpuRegister(Tokenizer& tokenizer, MipsRegisterValue& reg, int size);
	bool parseVfpuControlRegister(Tokenizer& tokenizer, MipsRegisterValue& reg);
	bool parseImmediate(Tokenizer& tokenizer, Expression& dest);
	bool parseVcstParameter(Tokenizer& tokenizer, int& result);
	bool parseVfpuVrot(Tokenizer& tokenizer, int& result, int size);
	bool parseVfpuCondition(Tokenizer& tokenizer, int& result);
	bool parseVpfxsParameter(Tokenizer& tokenizer, int& result);
	bool parseVpfxdParameter(Tokenizer& tokenizer, int& result);
	bool parseCop2BranchCondition(Tokenizer& tokenizer, int& result);
	bool parseWb(Tokenizer& tokenizer);

	bool decodeImmediateSize(const u8*& encoding, MipsImmediateType& dest);
	bool decodeCop2BranchCondition(const std::wstring& text, size_t& pos, int& result);
	bool decodeVfpuType(const std::wstring& name, size_t& pos, int& dest);
	bool decodeOpcode(const std::wstring& name, const tMipsOpcode& opcode);

	void setOmittedRegisters(const tMipsOpcode& opcode);
	bool matchSymbol(Tokenizer& tokenizer, wchar_t symbol);
	bool parseParameters(Tokenizer& tokenizer, const tMipsOpcode& opcode);

	MipsRegisterData registers;
	MipsImmediateData immediate;
	MipsOpcodeData opcodeData;
	bool hasFixedSecondaryImmediate;
};