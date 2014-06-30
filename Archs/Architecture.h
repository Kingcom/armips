#pragma once

class IElfRelocator;

enum class Endianness { Big, Little };

class CArchitecture
{
public:
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args) = 0;
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args) = 0;
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
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args);
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args);
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual int GetWordSize();
	virtual IElfRelocator* getElfRelocator();
	virtual Endianness getEndianness() { return Endianness::Little; };
};

extern CInvalidArchitecture InvalidArchitecture;
