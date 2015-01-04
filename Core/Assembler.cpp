#include "stdafx.h"
#include "Assembler.h"
#include "Core/Common.h"
#include "Commands/CAssemblerCommand.h"
#include "Commands/CAssemblerLabel.h"
#include "CMacro.h"
#include "Util/CommonClasses.h"
#include "Core/Directives.h"
#include "Archs/MIPS/Mips.h"
#include "Core/FileManager.h"
#include "Archs/ARM/Arm.h"
#define ASSEMBLER_MACRO_NESTING_LEVEL		128
#define ASSEMBLER_INCLUDE_NESTING_LEVEL		64

typedef struct {
	std::wstring buffer;
	std::wstring name;
	std::wstring params;
} tTextData;

inline bool CheckEndLine(std::wstring& string, int pos)
{
	if (pos >= (int)string.size()) return true;

	if (string[pos] == 0) return true;
	if (string[pos] == '\n') return true;
	if (string[pos] == ';') return true;
	if (pos+1 < (int)string.size() && string[pos+0] == '/' && string[pos+1] == '/') return true;
	return false;
}

bool GetLine(TextFile& Input, std::wstring& dest)
{
	std::wstring Buffer = Input.readLine();
	dest = L"";

	if (Input.hasError())
		Logger::printError(Logger::Warning,Input.getErrorText());

	int InputPos = 0;

	while (InputPos < (int)Buffer.size() && (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ')) InputPos++;
	while (InputPos < (int)Buffer.size() && CheckEndLine(Buffer,InputPos) == false)
	{
		switch (Buffer[InputPos])
		{
		case ' ':
		case '\t':
			while (InputPos < (int)Buffer.size() && (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ')) InputPos++;
			if (CheckEndLine(Buffer,InputPos) == true)
			{
				return true;
			}
			dest += ' ';
			break;
		case '"':	// string
			dest += towlower(Buffer[InputPos++]);
			while (InputPos < (int)Buffer.size() && Buffer[InputPos] != '"')
			{
				if (InputPos+1 < (int)Buffer.size() && Buffer[InputPos] == '\\' && Buffer[InputPos+1] == '"')
				{
					dest += '\\';
					dest += '"';
					InputPos += 2;
					continue;
				}
				if (InputPos+1 < (int)Buffer.size() && Buffer[InputPos] == '\\' && Buffer[InputPos+1] == '\\')
				{
					dest += '\\';
					dest += '\\';
					InputPos += 2;
					continue;
				}
				if (Buffer[InputPos] == '\n' || Buffer[InputPos] == 0)
				{
					Logger::printError(Logger::Error,L"Unexpected end of line in string constant");
					return false;
				}
				dest += Buffer[InputPos++];
			}
			if (InputPos == (int)Buffer.size())
			{
				Logger::printError(Logger::Error,L"Unexpected end of line in string constant");
				return false;
			}
			dest += towlower(Buffer[InputPos++]);
			break;
		case '\'':
			if (Buffer[InputPos+2] == '\'')
			{
				dest += Buffer[InputPos++];
				dest += Buffer[InputPos++];
				dest += Buffer[InputPos++];
			} else {
				Logger::printError(Logger::Error,L"Invalid character constant");
				return false;
			}
			break;
		default:
			dest += towlower(Buffer[InputPos++]);
			break;
		}
	}

	return true;
}

bool CheckEquLabel(std::wstring& str)
{
	size_t s = str.find(L" equ ");
	if (s == std::string::npos) s = str.find(L":equ ");
	if (s != std::string::npos)
	{
		std::wstring name = str.substr(0,s);
		if (name.back() == ':') name.pop_back();

		if (Global.symbolTable.isValidSymbolName(name) == false)
		{
			Logger::printError(Logger::Error,L"Invalid equation name %s",name);
			return true;
		}

		if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
		{
			Logger::printError(Logger::Error,L"Equation name %s already defined",name);
			return true;
		}
		
		std::wstring replacement = str.substr(s+5);
		Global.symbolTable.addEquation(name,Global.FileInfo.FileNum,Global.Section,replacement);
		return true;
	}

	return false;
}

std::wstring checkLabel(std::wstring& str, bool AllLocal)
{
	int pos = 0;

	while (pos < (int)str.size() && str[pos] != ' ' && str[pos] != 0)
	{
		if (str[pos] == ':')
		{
			std::wstring name = str.substr(0,pos);
			if (AllLocal == true && Global.symbolTable.isGlobalSymbol(name))
				name = L"@@" + name;
			
			addAssemblerLabel(name);
			return str.substr(pos+1);
		}
		pos++;
	}

	return str;
}

void splitLine(std::wstring& line, std::wstring& name, std::wstring& arguments)
{
	int linePos = 0;
	name = L"";
	arguments = L"";

	while (linePos < (int)line.size() && (line[linePos] == ' ' || line[linePos] == '\t')) linePos++;
	while (linePos < (int)line.size() && line[linePos] != ' ')
	{
		if (line[linePos]  == 0)
		{
			return;
		}
		name += line[linePos++];
	}
	
	while (linePos < (int)line.size() && (line[linePos] == ' ' || line[linePos] == '\t')) linePos++;

	while (linePos < (int)line.size() && line[linePos] != 0)
	{
		arguments += line[linePos++];
	}
}

void AddFileName(char* FileName)
{
	Global.FileInfo.FileNum = (int) Global.FileInfo.FileList.GetCount();
	Global.FileInfo.FileList.AddEntry(FileName);
	Global.FileInfo.LineNumber = 0;
}

void InsertMacro(CMacro* Macro, std::wstring& Args)
{
	tTextData Text;
	ArgumentList Arguments;

	splitArguments(Arguments,Args);

	if ((int)Arguments.size() != Macro->getArgumentCount())
	{
		Logger::printError(Logger::Error,L"%s macro arguments (%d vs %d)",
			(int)Arguments.size() > Macro->getArgumentCount() ? L"Too many" : L"Not enough",
			Arguments.size(),Macro->getArgumentCount());
		return;
	}

	Global.MacroNestingLevel++;
	if (Global.MacroNestingLevel == ASSEMBLER_MACRO_NESTING_LEVEL)
	{
		Logger::printError(Logger::Error,L"Maximum macro nesting level reached");
		return;
	}

	size_t MacroCounter = Macro->getIncreaseCounter();

	for (size_t i = 0; i < Macro->getLineCount(); i++)
	{
		Text.buffer = Macro->getLine(i,Arguments,MacroCounter);
		Text.buffer = Global.symbolTable.insertEquations(Text.buffer,Global.FileInfo.FileNum,Global.Section);

		if (CheckEquLabel(Text.buffer) == false)
		{
			Text.buffer = checkLabel(Text.buffer,false);
			splitLine(Text.buffer,Text.name,Text.params);
			if (Text.name.size() == 0) continue;

			bool macro = false;
			for (size_t i = 0; i < Global.Macros.size(); i++)
			{
				if (Text.name.compare(Global.Macros[i]->getName()) == 0)
				{
					InsertMacro(Global.Macros[i],Text.params);
					macro = true;
				}
			}
			if (macro == true) continue;

			if (Arch->AssembleDirective(Text.name,Text.params) == false)
			{
				Arch->AssembleOpcode(Text.name,Text.params);
			}
		}
	}
	Global.MacroNestingLevel--;
}

bool ParseMacro(TextFile& Input, std::wstring& opcodeName, std::wstring& Args)
{
	if (opcodeName.compare(L".macro") == 0)
	{
		parseMacroDefinition(Input,Args);
		return true;
	}

	for (size_t i = 0; i < Global.Macros.size(); i++)
	{
		if (opcodeName.compare(Global.Macros[i]->getName()) == 0)
		{
			Global.MacroNestingLevel = 0;
			InsertMacro(Global.Macros[i],Args);
			return true;
		}
	}
	return false;
}

void parseMacroDefinition(TextFile& Input, std::wstring& Args)
{
	tTextData Text;
	ArgumentList Arguments;

	splitArguments(Arguments,Args);

	CMacro* Macro = new CMacro();
	Macro->loadArguments(Arguments);

	while (true)
	{
		if (Input.atEnd())
		{
			Logger::printError(Logger::Error,L"Unexpected end of line in macro definition");
			return;
		}
		Global.FileInfo.LineNumber++;
		if (GetLine(Input,Text.buffer) == false) continue;
		if (Text.buffer.empty()) continue;
		splitLine(Text.buffer,Text.name,Text.params);
		if (Text.name.compare(L".endmacro") == 0) break;
		Macro->addLine(Text.buffer);
	}

	for (size_t i = 0; i < Global.Macros.size(); i++)
	{
		if (Macro->getName().compare(Global.Macros[i]->getName()) == 0)
		{
			Logger::printError(Logger::Error,L"Macro \"%s\" already defined",Macro->getName());
			delete Macro;
			return;
		}
	}

	Global.Macros.push_back(Macro);
}

void parseFile(TextFile& input)
{
	tTextData Text;

	while (!input.atEnd())
	{
		Global.FileInfo.LineNumber++;
		Global.FileInfo.TotalLineCount++;

		if (GetLine(input,Text.buffer) == false) continue;
		if (Text.buffer.size() == 0) continue;
		
		Text.buffer = Global.symbolTable.insertEquations(Text.buffer,Global.FileInfo.FileNum,Global.Section);

		if (CheckEquLabel(Text.buffer) == false)
		{
			Text.buffer = checkLabel(Text.buffer,false);
			splitLine(Text.buffer,Text.name,Text.params);
			if (Text.name.empty()) continue;

			if (ParseMacro(input,Text.name,Text.params) == true) continue;
			if (Arch->AssembleDirective(Text.name,Text.params) == false)
			{
				Arch->AssembleOpcode(Text.name,Text.params);
			}
		}

		if (Logger::hasFatalError())
			return;
	}
	
	Logger::printQueue();
}

void LoadAssemblyFile(const std::wstring& fileName, TextFile::Encoding encoding)
{
	AddFileName((char*)convertWStringToUtf8(fileName).c_str());
	Global.IncludeNestingLevel++;

	if (Global.IncludeNestingLevel == ASSEMBLER_INCLUDE_NESTING_LEVEL)
	{
		Logger::printError(Logger::Error,L"Maximum include nesting level reached");
		return;
	}

	TextFile input;
	if (input.open(fileName,TextFile::Read,encoding) == false)
	{
		Logger::printError(Logger::Error,L"Could not open file");
		return;
	}

	parseFile(input);

	Global.IncludeNestingLevel--;
	input.close();
}

void LoadAssemblyContent(const std::wstring& content)
{
	AddFileName("Memory");
	g_fileManager->openFile(Global.memoryFile,true);

	TextFile input;
	input.openMemory(content);
	parseFile(input);
}

bool EncodeAssembly()
{
	bool Revalidate;
	Arch->Pass2();
	int validationPasses = 0;
	do	// loop until everything is constant
	{
		Global.validationPasses = validationPasses;
		Logger::clearQueue();
		Revalidate = false;

		if (validationPasses >= 100)
		{
			Logger::queueError(Logger::Error,L"Stuck in infinite validation loop");
			break;
		}

		g_fileManager->reset();
		Arch->Revalidate();
		Global.Section = 0;
#ifdef _DEBUG
		if (!Logger::isSilent())
			printf("Validate %d...\n",validationPasses);
#endif

		if (Global.memoryMode)
			g_fileManager->openFile(Global.memoryFile,true);

		for (size_t i = 0; i < Global.Commands.size(); i++)
		{
			if (Global.Commands[i]->IsConditional() == false)
			{
				if ((Global.Commands[i]->IsPool() == true) && (Global.conditionData.conditionTrue() == false))
				{
					Arm.NextPool();
				}
				if (Global.conditionData.conditionTrue() == false)
					continue;
			}

			Global.Commands[i]->SetFileInfo();
			Global.areaData.checkAreas();

			if (Global.Commands[i]->Validate() == true)
				Revalidate = true;
		}
		if (Global.conditionData.activeConditions() != 0)
			Logger::queueError(Logger::Error,L"One or more if statements not terminated");
		validationPasses++;
	} while (Revalidate == true);

	Logger::printQueue();
	if (Logger::hasError() == true)
	{
		return false;
	}

#ifdef _DEBUG
	if (!Logger::isSilent())
		printf("Encode...\n");
#endif

	// and finally encode
	Global.Section = 0;
	Global.tempData.start();

	if (Global.memoryMode)
		g_fileManager->openFile(Global.memoryFile,false);

	for (size_t i = 0; i < Global.Commands.size(); i++)
	{
		if (Global.Commands[i]->IsConditional() == false && Global.conditionData.conditionTrue() == false)
		{
			continue;
		}

		Global.Commands[i]->SetFileInfo();
		Global.Commands[i]->writeTempData(Global.tempData);
		Global.Commands[i]->writeSymData(Global.symData);
		Global.Commands[i]->Encode();
	}

	Global.tempData.end();
	Global.symData.write();

	if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning,L"File not closed");
		g_fileManager->closeFile();
	}

	return true;
}

bool runArmips(ArmipsArguments& arguments)
{
	// initialize and reset global data
	Global.Radix = 10;
	Global.Revalidate = true;
	Global.Section = 0;
	Global.nocash = false;
	Global.IncludeNestingLevel = 0;
	Global.MacroNestingLevel = 0;
	Global.FileInfo.FileCount = 0;
	Global.FileInfo.TotalLineCount = 0;
	Global.DebugMessages = 0;
	Global.relativeInclude = false;
	Global.validationPasses = 0;
	Arch = &InvalidArchitecture;

	Logger::clear();
	Global.symData.clear();
	Global.Table.clear();
	Global.symbolTable.clear();
	Global.tempData.clear();
	Global.conditionData.clear();
	Global.areaData.clear();
	Global.Commands.clear();
	Global.Macros.clear();

	Global.FileInfo.FileList.Clear();
	Global.FileInfo.FileCount = 0;
	Global.FileInfo.TotalLineCount = 0;
	Global.FileInfo.LineNumber = 0;
	Global.FileInfo.FileNum = 0;

	Arm.clear();

	// process arguments
	Logger::setSilent(arguments.silent);
	Logger::setErrorOnWarning(arguments.errorOnWarning);

	if (!arguments.symFileName.empty())
		Global.symData.setNocashSymFileName(arguments.symFileName,arguments.symFileVersion);

	if (!arguments.tempFileName.empty())
		Global.tempData.setFileName(arguments.tempFileName);

	for (size_t i = 0; i < arguments.equList.size(); i++)
	{
		std::wstring equline = arguments.equList.at(i);
		CheckEquLabel(equline);
	}

	Global.symbolTable.addLabels(arguments.labels);

	// run assembler
	switch (arguments.mode)
	{
	case ArmipsMode::FILE:
		Global.memoryMode = false;
		LoadAssemblyFile(arguments.inputFileName);
		break;
	case ArmipsMode::MEMORY:
		Global.memoryMode = true;
		Global.memoryFile = arguments.memoryFile;
		LoadAssemblyContent(arguments.content);
		break;
	}

	bool result = !Logger::hasError();
	if (result == true)
		result = EncodeAssembly();
	else if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning,L"File not closed");
		g_fileManager->closeFile();
	}

	// return errors
	if (arguments.errorsResult != NULL)
	{
		StringList errors = Logger::getErrors();
		for (size_t i = 0; i < errors.size(); i++)
			arguments.errorsResult->push_back(errors[i]);
	}

	// cleanup
	for (size_t i = 0; i < Global.Commands.size(); i++)
		delete Global.Commands[i];	

	for (size_t i = 0; i < Global.Macros.size(); i++)
		delete Global.Macros[i];

	Global.Commands.clear();
	Global.Macros.clear();

	return result;
}
