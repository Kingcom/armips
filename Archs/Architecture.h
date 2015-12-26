#pragma once

class IElfRelocator;
class CAssemblerCommand;
class Tokenizer;
class Parser;

enum class Endianness { Big, Little };

class CArchitecture
{
public:
	virtual CAssemblerCommand* parseDirective(Parser& parserr) { return nullptr; };
	virtual CAssemblerCommand* parseOpcode(Parser& parser) { return nullptr; };
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual int GetWordSize() = 0;
	virtual IElfRelocator* getElfRelocator() = 0;
	virtual Endianness getEndianness() = 0;
};

class CInvalidArchitecture: public CArchitecture
{
public:
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual int GetWordSize();
	virtual IElfRelocator* getElfRelocator();
	virtual Endianness getEndianness() { return Endianness::Little; };
};

extern CInvalidArchitecture InvalidArchitecture;
