#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "ArmOpcodes.h"
#include "Arm.h"
#include "Core/MathParser.h"

typedef struct {
	struct {
		unsigned char c,a;
		bool s,x,y;
		unsigned int NewEncoding;
		char NewType;
		bool UseNewEncoding;
		bool UseNewType;
	} Opcode;
	struct {
		unsigned char Type;
		bool ShiftByRegister;
		bool UseShift;
		tArmRegisterInfo reg;
		CExpressionCommandList ShiftExpression;
		int ShiftAmount;
		unsigned char FinalType;
		int FinalShiftAmount;
		bool UseFinal;
	} Shift;
	struct {
		bool spsr;
		int field;
	} PsrData;
	struct CopData {
		tArmRegisterInfo cd;	// cop register d
		tArmRegisterInfo cn;	// cop register n
		tArmRegisterInfo cm;	// cop register m
		tArmRegisterInfo pn;	// cop number
		CExpressionCommandList CpopExpression;	// cp opc number
		CExpressionCommandList CpinfExpression;	// cp information
		int Cpop;
		int Cpinf;
	} CopData;
	tArmRegisterInfo rs;
	tArmRegisterInfo rm;
	tArmRegisterInfo rd;
	tArmRegisterInfo rn;
	bool psr;
	bool writeback;
	bool SignPlus;
	bool negative;
	CExpressionCommandList ImmediateExpression;
	int Immediate;
	int ImmediateBitLen;
	int OriginalImmediate;
	int rlist;
	char RlistStr[64];
} tArmOpcodeVariables;

class CArmInstruction: public CAssemblerCommand
{
public:
	CArmInstruction();
//	~CArmInstruction();
	bool Load(char* Name, char* Params);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	void FormatOpcode(char* Dest, const char* Source);
	void FormatInstruction(const char* encoding, char* dest);
	bool ParseOpcode(char* Encoding, char* Line);
	bool LoadEncoding(const tArmOpcode& SourceOpcode, char* Line);
	bool ParseShift(char*& Line, int mode);
	void FormatInstruction(char* encoding,tArmOpcodeVariables& Vars, char* dest);
	void WriteInstruction(unsigned int encoding);
	tArmOpcodeVariables Vars;
	tArmOpcode Opcode;
	bool NoCheckError;
	bool Loaded;
	u64 RamPos;
};
