#include "stdafx.h"
#include "PsxRelocator.h"
#include "Core/Misc.h"
#include "Core/Common.h"
#include <map>
#include "Util/CRC.h"
#include "Core/FileManager.h"

struct PsxLibEntry
{
	std::string name;
	ByteArray data;
};

const unsigned char psxObjectFileMagicNum[6] = { 'L', 'N', 'K', '\x02', '\x2E', '\x07' };

std::vector<PsxLibEntry> loadPsxLibrary(const std::wstring& inputName)
{
	ByteArray input = ByteArray::fromFile(inputName);
	std::vector<PsxLibEntry> result;

	if (memcmp(input.data(),psxObjectFileMagicNum,sizeof(psxObjectFileMagicNum)) == 0)
	{
		PsxLibEntry entry;
		entry.name = "";
		entry.data = input;
		result.push_back(entry);
		return result;
	}

	return result;
}

int PsxRelocator::loadString(ByteArray& data, int pos, std::wstring& dest)
{
	dest = L"";
	int len = data[pos++];

	for (int i = 0; i < len; i++)
	{
		dest += tolower(data[pos++]);
	}

	return len+1;
}

bool PsxRelocator::parseObject(ByteArray data, PsxRelocatorFile& dest)
{
	if (memcmp(data.data(),psxObjectFileMagicNum,sizeof(psxObjectFileMagicNum)) != 0)
		return false;

	int pos = 6;

	std::vector<PsxSegment>& segments = dest.segments;
	std::vector<PsxSymbol>& syms = dest.symbols;

	int activeSegment = -1;
	while (pos < data.size())
	{
		switch (data[pos])
		{
		case 0x10:	// segment definition
			{
				PsxSegment seg;
				seg.id = data.getDoubleWord(pos+1);
				segments.push_back(seg);
				pos += 5;
			}
			break;
		case 0x08:	// segment name
			{
				int nameLen = data[pos+1];
				std::wstring& name = segments[segments.size()-1].name;
				pos += 1 + loadString(data,pos+1,name);
			}
			break;
		case 0x1C:	// source file name
			pos += data[pos+3]+4;
			break;

		case 0x06:	// set segment id
			{
				while (data[pos] == 0x06)
				{
					int id = data.getWord(pos+1);
					pos += 3;
				
					int num = -1;
					for (size_t i = 0; i < segments.size(); i++)
					{
						if (segments[i].id == id)
						{
							num = i;
							break;
						}
					}

					activeSegment = num;
				}

				switch (data[pos])
				{
				case 0x02:	// segment data
					{
						int size = data.getWord(pos+1);
						pos += 3;

						segments[activeSegment].bss = false;
						segments[activeSegment].data = data.mid(pos,size);
						pos += size;
					}
					break;
				case 0x08:	// segment size
					{
						int size = data.getDoubleWord(pos+1);
						pos += 5;
						segments[activeSegment].bss = true;
						segments[activeSegment].data.reserveBytes(size);
					}
					break;
				}
			}
			break;
		case 0x0A:	// relocation data
			{
				int type = data[pos+1];
				pos += 2;

				PsxRelocation rel;
				switch (type)
				{
				case 0x10:	// 32 bit word
					rel.type = PsxRelocationType::WordLiteral;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x4A:	// jal
					rel.type = PsxRelocationType::FunctionCall;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x52:	// upper immerdiate
					rel.type = PsxRelocationType::UpperImmediate;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x54:	// lower immediate (add)
					rel.type = PsxRelocationType::LowerImmediate;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				default:
					return false;
				}

				int otherType = data[pos++];
				switch (otherType)
				{
				case 0x02:	// reference to symbol with id num
					rel.refType = PsxRelocationRefType::SymblId;
					rel.referenceId = data.getWord(pos);
					pos += 2;
					break;
				case 0x2C:	// ref to other segment?
					rel.refType = PsxRelocationRefType::SegmentOffset;
					if (data[pos++] != 0x04)
					{
						return false;
					}
					
					rel.referenceId = data.getWord(pos);	// segment id
					pos += 2;
					
					if (data[pos++] != 0x00)
					{
						return false;
					}

					rel.referencePos = data.getDoubleWord(pos);
					pos += 4;
					break;
				default:
					return false;
				}

				segments[activeSegment].relocations.push_back(rel);
			}
			break;
		case 0x12:	// internal symbol
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::Internal;
				sym.segment = data.getWord(pos+1);
				sym.offset = data.getDoubleWord(pos+3);
				pos += 7 + loadString(data,pos+7,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x0E:	// external symbol
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::External;
				sym.id = data.getWord(pos+1);
				pos += 3 + loadString(data,pos+3,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x30:	// bss symbol?
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::BSS;
				sym.id = data.getWord(pos+1);
				sym.segment = data.getWord(pos+3);
				sym.size = data.getDoubleWord(pos+5);
				pos += 9 + loadString(data,pos+9,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x0C:	// internal with id
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::InternalID;
				sym.id = data.getWord(pos+1);
				sym.segment = data.getWord(pos+3);
				sym.offset = data.getDoubleWord(pos+5);
				pos += 9 + loadString(data,pos+9,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x4A:	// function
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::Function;
				sym.segment = data.getWord(pos+1);
				sym.offset = data.getDoubleWord(pos+3);
				pos += 0x1D + loadString(data,pos+0x1D,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x4C:	// function size
			pos += 11;
			break;
		case 0x3C:	// ??
			pos += 3;
			break;
		case 0x00:	// ??
			pos++;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool PsxRelocator::init(const std::wstring& inputName)
{
	auto inputFiles = loadPsxLibrary(inputName);
	if (inputFiles.size() == 0)
	{
		Logger::printError(Logger::Error,L"Could not load library");
		return false;
	}

	reloc = new MipsElfRelocator();

	for (PsxLibEntry& entry: inputFiles)
	{
		PsxRelocatorFile file;
		if (parseObject(entry.data,file) == false)
		{
			Logger::printError(Logger::Error,L"Could not load object file %s",entry.name.c_str());
			return false;
		}

		// init symbols
		bool error = false;
		for (PsxSymbol& sym: file.symbols)
		{
			sym.label = Global.symbolTable.getLabel(sym.name,-1,-1);
			if (sym.label == NULL)
			{
				Logger::printError(Logger::Error,L"Invalid label name \"%s\"",sym.name.c_str());
				error = true;
				continue;
			}

			if (sym.label->isDefined() && sym.type != PsxSymbolType::External)
			{
				Logger::printError(Logger::Error,L"Label \"%s\" already defined",sym.name.c_str());
				error = true;
				continue;
			}
		}

		files.push_back(file);
	}

	return true;
}

bool PsxRelocator::relocateFile(PsxRelocatorFile& file, int& relocationAddress)
{
	std::map<int,int> relocationOffsets;
	std::map<int,int> symbolOffsets;
	int start = relocationAddress;

	// assign addresses to segments
	for (PsxSegment& seg: file.segments)
	{
		int index = seg.id;
		int size = seg.data.size();
		
		relocationOffsets[index] = relocationAddress;
		relocationAddress += size;
	}
	
	// parse/add/relocate symbols
	for (PsxSymbol& sym: file.symbols)
	{
		int pos;
		switch (sym.type)
		{
		case PsxSymbolType::Internal:
		case PsxSymbolType::Function:
			sym.label->setValue(relocationOffsets[sym.segment]+sym.offset);
			sym.label->setDefined(true);
			break;
		case PsxSymbolType::InternalID:
			pos = relocationOffsets[sym.segment]+sym.offset;
			sym.label->setValue(pos);
			sym.label->setDefined(true);
			symbolOffsets[sym.id] = pos;
			break;
		case PsxSymbolType::BSS:
			sym.label->setValue(relocationAddress);
			sym.label->setDefined(true);
			symbolOffsets[sym.id] = relocationAddress;
			relocationAddress += sym.size;
			break;
		case PsxSymbolType::External:
			if (sym.label->isDefined() == false)
			{
				Logger::queueError(Logger::Error,L"Undefined external symbol %s in file %s",sym.name.c_str(),file.name.c_str());
				return false;
			}
			
			symbolOffsets[sym.id] = sym.label->getValue();
			break;
		}
	}

	int dataStart = outputData.size();
	outputData.reserveBytes(relocationAddress-start);

	// load code and data
	for (PsxSegment& seg: file.segments)
	{
		if (seg.bss)
		{
			// reserveBytes initialized the data to 0 already
			continue;
		}
		
		// relocate
		ByteArray sectionData = seg.data;
		for (PsxRelocation& rel: seg.relocations)
		{
			RelocationData relData;
			int pos = rel.segmentOffset;
			relData.opcode = sectionData.getDoubleWord(pos);

			switch (rel.refType)
			{
			case PsxRelocationRefType::SymblId:
				relData.relocationBase = symbolOffsets[rel.referenceId];
				break;
			case PsxRelocationRefType::SegmentOffset:
				relData.relocationBase = relocationOffsets[rel.referenceId] + rel.referencePos;
				break;
			}
			
			switch (rel.type)
			{
			case PsxRelocationType::WordLiteral:
				reloc->relocateOpcode(R_MIPS_32,relData);
				break;
			case PsxRelocationType::UpperImmediate:
				reloc->relocateOpcode(R_MIPS_HI16,relData);
				break;
			case PsxRelocationType::LowerImmediate:
				reloc->relocateOpcode(R_MIPS_LO16,relData);
				break;
			case PsxRelocationType::FunctionCall:
				reloc->relocateOpcode(R_MIPS_26,relData);
				break;
			}

			sectionData.replaceDoubleWord(pos,relData.opcode);
		}

		int arrayStart = dataStart+relocationOffsets[seg.id]-start;
		memcpy(outputData.data(arrayStart),sectionData.data(),sectionData.size());
	}

	return true;
}

bool PsxRelocator::relocate(int& memoryAddress)
{
	int oldCrc = getCrc32(outputData.data(),outputData.size());
	outputData.clear();
	dataChanged = false;

	bool error = false;
	int start = memoryAddress;

	for (PsxRelocatorFile& file: files)
	{
		if (relocateFile(file,memoryAddress) == false)
			error = true;
	}
	
	int newCrc = getCrc32(outputData.data(),outputData.size());
	if (oldCrc != newCrc)
		dataChanged = true;

	memoryAddress -= start;
	return !error;
}


void PsxRelocator::writeSymbols(SymbolData& symData)
{
	for (PsxRelocatorFile& file: files)
	{
		for (PsxSymbol& sym: file.symbols)
		{
			if (sym.type != PsxSymbolType::External)
				symData.addLabel(sym.label->getValue(),sym.name.c_str());
		}
	}
}

//
// DirectivePsxObjImport
//

DirectivePsxObjImport::DirectivePsxObjImport(ArgumentList& args)
{
	if (rel.init(args[0].text))
	{
	}
}

bool DirectivePsxObjImport::Validate()
{
	int memory = g_fileManager->getVirtualAddress();
	rel.relocate(memory);
	g_fileManager->advanceMemory(memory);
	return rel.hasDataChanged();
}

void DirectivePsxObjImport::Encode()
{
	ByteArray& data = rel.getData();
	g_fileManager->write(data.data(),data.size());
}

void DirectivePsxObjImport::writeSymData(SymbolData& symData)
{
	rel.writeSymbols(symData);
}