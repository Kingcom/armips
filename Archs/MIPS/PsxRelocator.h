#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Util/ByteArray.h"

#include <memory>
#include <string>
#include <vector>

class Label;
class MipsElfRelocator;

enum class PsxRelocationType { WordLiteral, UpperImmediate, LowerImmediate, FunctionCall };
enum class PsxRelocationRefType { SymblId, SegmentOffset };

struct PsxRelocation
{
	PsxRelocationType type;
	PsxRelocationRefType refType;
	int segmentOffset = 0;
	int referenceId = 0;
	int referencePos = 0;
	int relativeOffset = 0;
	int filePos = 0;
};

struct PsxSegment
{
	std::string name;
	int id;
	ByteArray data;
	std::vector<PsxRelocation> relocations;
};


enum class PsxSymbolType { Internal, InternalID, External, BSS, Function };

struct PsxSymbol
{
	PsxSymbolType type;
	std::string name;
	int segment;
	int offset;
	int id;
	int size;
	std::shared_ptr<Label> label;
};

struct PsxRelocatorFile
{
	std::string name;
	std::vector<PsxSegment> segments;
	std::vector<PsxSymbol> symbols;
};

class PsxRelocator
{
public:
	bool init(const fs::path& inputName);
	bool relocate(int& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	const ByteArray& getData() const { return outputData; };
	void writeSymbols(SymbolData& symData) const;
private:
	size_t loadString(ByteArray& data, size_t pos, std::string& dest);
	bool parseObject(ByteArray data, PsxRelocatorFile& dest);
	bool relocateFile(PsxRelocatorFile& file, int& relocationAddress);
	
	ByteArray outputData;
	std::vector<PsxRelocatorFile> files;
	MipsElfRelocator* reloc;
	bool dataChanged;
};

class DirectivePsxObjImport: public CAssemblerCommand
{
public:
	DirectivePsxObjImport(const fs::path& fileName);
	~DirectivePsxObjImport() { };
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override;
	inline bool isSuccessfullyImported() { return success; };
private:
	bool success;
	PsxRelocator rel;
};
