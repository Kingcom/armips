#include "stdafx.h"
#include "Assembler.h"
#include "Core/Common.h"
#include "Commands/CAssemblerCommand.h"
#include "Commands/CAssemblerLabel.h"
#include "CMacro.h"
#include "Util/CommonClasses.h"
#include "Core/Directives.h"
#include "Archs/MIPS/Mips.h"

#define ASSEMBLER_MACRO_NESTING_LEVEL		128
#define ASSEMBLER_INCLUDE_NESTING_LEVEL		64

typedef struct {
	char Buffer[2048];
	char Name[128];
	char Params[1920];
} tTextData;

inline bool CheckEndLine(char* string)
{
	if (*string == 0) return true;
	if (*string == '\n') return true;
	if (*string == ';') return true;
	if (string[0] == '/' && string[1] == '/') return true;
	return false;
}

bool GetLine(FILE*& Input)
{
	char Buffer[2048],TempString[2048];

	Global.Arguments.Clear();
	fgets(Buffer,2048,Input);
	if (feof(Input))
	{
		return true;
	}

	int InputPos = 0;
	while (true)
	{
		int TempPos = 0;
		while (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ') InputPos++;
		if (CheckEndLine(&Buffer[InputPos]) == true) break;

		bool endline = false;
		while (endline == false &&  CheckEndLine(&Buffer[InputPos]) == false)
		{
			switch (Buffer[InputPos])
			{
			case ' ':
			case '\t':
				while (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ') InputPos++;
				if (Buffer[InputPos] != ',' || CheckEndLine(&Buffer[InputPos]) == true)
				{
					endline = true;
					break;
				}
				TempString[TempPos++] = ' ';
				break;
			case '"':	// string
				TempString[TempPos++] = tolower(Buffer[InputPos++]);
				while (Buffer[InputPos] != '"')
				{
					if (Buffer[InputPos] == '\n' || Buffer[InputPos] == 0)
					{
						PrintError(ERROR_ERROR,"Unexpected end of line in string constant");
						return false;
					}
					TempString[TempPos++] = Buffer[InputPos++];
				}
				TempString[TempPos++] = tolower(Buffer[InputPos++]);
				break;
			case '\'':
				if (Buffer[InputPos+2] == '\'')
				{
					TempString[TempPos++] = Buffer[InputPos++];
					TempString[TempPos++] = Buffer[InputPos++];
					TempString[TempPos++] = Buffer[InputPos++];
				} else {
					PrintError(ERROR_ERROR,"Invalid character constant");
					return false;
				}
				break;
			default:
				TempString[TempPos++] = tolower(Buffer[InputPos++]);
				break;
			}
		}
		TempString[TempPos] = 0;
		Global.Arguments.AddEntry(TempString);
	}
}

bool GetLine(FILE*& Input, char* dest)
{
	char Buffer[2048];
	char* s = fgets(Buffer,2048,Input);
	if (feof(Input) && s == NULL)
	{
		dest[0] = 0;
		return true;
	}

	int InputPos = 0;
	int OutputPos = 0;

	while (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ') InputPos++;
	while (CheckEndLine(&Buffer[InputPos]) == false)
	{
		switch (Buffer[InputPos])
		{
		case ' ':
		case '\t':
			while (Buffer[InputPos] == '\t' || Buffer[InputPos] == ' ') InputPos++;
			if (CheckEndLine(&Buffer[InputPos]) == true)
			{
				dest[OutputPos] = 0;
				return true;
			}
			dest[OutputPos++] = ' ';
			break;
		case '"':	// string
			dest[OutputPos++] = tolower(Buffer[InputPos++]);
			while (Buffer[InputPos] != '"')
			{
				if (Buffer[InputPos] == '\\' && Buffer[InputPos+1] == '"')
				{
					dest[OutputPos++] = '\\';
					dest[OutputPos++] = '"';
					InputPos += 2;
					continue;
				}
				if (Buffer[InputPos] == '\\' && Buffer[InputPos+1] == '\\')
				{
					dest[OutputPos++] = '\\';
					dest[OutputPos++] = '\\';
					InputPos += 2;
					continue;
				}
				if (Buffer[InputPos] == '\n' || Buffer[InputPos] == 0)
				{
					PrintError(ERROR_ERROR,"Unexpected end of line in string constant");
					return false;
				}
				dest[OutputPos++] = Buffer[InputPos++];
			}
			dest[OutputPos++] = tolower(Buffer[InputPos++]);
			break;
		case '\'':
			if (Buffer[InputPos+2] == '\'')
			{
				dest[OutputPos++] = Buffer[InputPos++];
				dest[OutputPos++] = Buffer[InputPos++];
				dest[OutputPos++] = Buffer[InputPos++];
			} else {
				PrintError(ERROR_ERROR,"Invalid character constant");
				return false;
			}
			break;
		default:
			dest[OutputPos++] = tolower(Buffer[InputPos++]);
			break;
		}
	}
	dest[OutputPos] = 0;
	return true;
}

bool CheckEquLabel(char* str)
{
	char* s = strstr(str," equ ");
	if (s == NULL) s = strstr(str,":equ ");	// check
	if (s != NULL)
	{
		int pos = 0;

		while (str[pos] != ':' && str[pos] != ' ') pos++;
		str[pos++] = 0;	// lˆschen f¸r sp‰ter

		std::wstring name = convertUtf8ToWString(str);
		if (Global.symbolTable.isValidSymbolName(name) == false)
		{
			PrintError(ERROR_ERROR,"Invalid equation name %s",str);
			return true;
		}

		if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
		{
			PrintError(ERROR_ERROR,"Equation name %s already defined",str);
			return true;
		}

		std::wstring replacement = convertUtf8ToWString(&s[5]);
		Global.symbolTable.addEquation(name,Global.FileInfo.FileNum,Global.Section,replacement);
		return true;
	}

	return false;
}

// TODO: RESTLICHE ROUTINEN AUCH ANPASSEN
int CheckLabel(char* str, bool AllLocal)
{
	char LabelName[128];
	int pos = 0;
	int NamePos = 0;

	while (str[pos] != ' ' && str[pos] != 0)
	{
		if (str[pos] == ':')
		{
			str[pos] = 0;
			if (AllLocal == true)
			{
				if (str[0] != '@' && str[1] != '@')
				{
					LabelName[NamePos++] = '@';
					LabelName[NamePos++] = '@';
				}
			}
			strcpy(&LabelName[NamePos],str);
			AddAssemblerLabel(LabelName);
			return pos+1;
		}
		pos++;
	}

	return 0;
}


void SplitLine(char* Line, char* Name, char* Arguments)
{
	while (*Line == ' ' || *Line == '\t') Line++;
	while (*Line != ' ')
	{
		if (*Line  == 0)
		{
			*Name = 0;
			*Arguments = 0;
			return;
		}
		*Name++ = *Line++;
	}
	*Name = 0;
	
	while (*Line == ' ' || *Line == '\t') Line++;

	while (*Line != 0)
	{
		*Arguments++ = *Line++;
	}
	*Arguments = 0;
}

void AddFileName(char* FileName)
{
	Global.FileInfo.FileNum = Global.FileInfo.FileList.GetCount();
	Global.FileInfo.FileList.AddEntry(FileName);
	Global.FileInfo.LineNumber = 0;
}

void InsertMacro(CMacro* Macro, char* Args)
{
	tTextData* Text = (tTextData*) malloc(sizeof(tTextData));
	CArgumentList Arguments;

	SplitArguments(Arguments,Args);

	if (Arguments.GetCount() != Macro->GetArgumentCount())
	{
		PrintError(ERROR_ERROR,"%s macro arguments (%d vs %d)",
			Arguments.GetCount() > Macro->GetArgumentCount() ? "Too many" : "Not enough",
			Arguments.GetCount(),Macro->GetArgumentCount());
		return;
	}

	Global.MacroNestingLevel++;
	if (Global.MacroNestingLevel == ASSEMBLER_MACRO_NESTING_LEVEL)
	{
		PrintError(ERROR_ERROR,"Maximum macro nesting level reached");
		return;
	}

	int MacroCounter = Macro->GetIncreaseCounter();

	for (int i = 0; i < Macro->GetLineCount(); i++)
	{
		Macro->GetLine(i,Arguments,Text->Buffer,MacroCounter);

		std::wstring wide = Global.symbolTable.insertEquations(convertUtf8ToWString(Text->Buffer),Global.FileInfo.FileNum,Global.Section);
		std::string normal = convertWStringToUtf8(wide);
		strcpy(Text->Buffer,normal.c_str());

		if (CheckEquLabel(Text->Buffer) == false)
		{
			int pos = CheckLabel(Text->Buffer,false);
			SplitLine(&Text->Buffer[pos],Text->Name,Text->Params);
			if (Text->Name[0] == 0) continue;

			bool macro = false;
			for (size_t i = 0; i < Global.Macros.size(); i++)
			{
				if (strcmp(Global.Macros[i]->GetName(),Text->Name) == 0)
				{
					InsertMacro(Global.Macros[i],Text->Params);
					macro = true;
				}
			}
			if (macro == true) continue;

			if (Arch->AssembleDirective(Text->Name,Text->Params) == false)
			{
				Arch->AssembleOpcode(Text->Name,Text->Params);
			}
		}
	}
	Global.MacroNestingLevel--;
	free(Text);
}

bool ParseMacro(FILE*& Input, char* OpcodeName, char* Args)
{
	if (strcmp(OpcodeName,".macro") == 0)	// macro definition
	{
		ParseMacroDefinition(Input,Args);
		return true;
	}

	for (size_t i = 0; i < Global.Macros.size(); i++)
	{
		if (strcmp(Global.Macros[i]->GetName(),OpcodeName) == 0)
		{
			Global.MacroNestingLevel = 0;
			InsertMacro(Global.Macros[i],Args);
			return true;
		}
	}
	return false;
}

void ParseMacroDefinition(FILE*& Input, char* Args)
{
	tTextData* Text = (tTextData*) malloc(sizeof(tTextData));
	CArgumentList Arguments;

	SplitArguments(Arguments,Args);

	CMacro* Macro = new CMacro();
	Macro->LoadArguments(Arguments);

	while (true)
	{
		if (feof(Input))
		{
			PrintError(ERROR_ERROR,"Unexpected end of line in macro definition");
			return;
		}
		Global.FileInfo.LineNumber++;
		if (GetLine(Input,Text->Buffer) == false) continue;
		if (Text->Buffer[0] == 0) continue;
		SplitLine(Text->Buffer,Text->Name,Text->Params);
		if (strcmp(Text->Name,".endmacro") == 0) break;
		Macro->AddLine(Text->Buffer);
	}

	for (size_t i = 0; i < Global.Macros.size(); i++)
	{
		if (strcmp(Macro->GetName(),Global.Macros[i]->GetName()) == 0)
		{
			PrintError(ERROR_ERROR,"Macro \"%s\" already defined",Macro->GetName());
			delete Macro;
			return;
		}
	}

	Global.Macros.push_back(Macro);
	free(Text);
}

void LoadAssemblyFile(char* FileName)
{
	tTextData* Text = (tTextData*) malloc(sizeof(tTextData));
	CStringList Arguments;
	int num = 0;

	AddFileName(FileName);
	Global.IncludeNestingLevel++;

	if (Global.IncludeNestingLevel == ASSEMBLER_INCLUDE_NESTING_LEVEL)
	{
		PrintError(ERROR_ERROR,"Maximum include nesting level reached");
		return;
	}

	FILE* Input = fopen(FileName,"r");
	if (CheckBom(Input) == false) return;

	while (!feof(Input))
	{
		Global.FileInfo.LineNumber++;
		Global.FileInfo.TotalLineCount++;

		if (GetLine(Input,Text->Buffer) == false) continue;
		if (Text->Buffer[0] == 0) continue;
		
		std::wstring wide = Global.symbolTable.insertEquations(convertUtf8ToWString(Text->Buffer),Global.FileInfo.FileNum,Global.Section);
		std::string normal = convertWStringToUtf8(wide);
		strcpy(Text->Buffer,normal.c_str());

		if (CheckEquLabel(Text->Buffer) == false)
		{
			int pos = CheckLabel(Text->Buffer,false);
			SplitLine(&Text->Buffer[pos],Text->Name,Text->Params);
			if (Text->Name[0] == 0) continue;

			if (ParseMacro(Input,Text->Name,Text->Params) == true) continue;
			if (Arch->AssembleDirective(Text->Name,Text->Params) == false)
			{
				Arch->AssembleOpcode(Text->Name,Text->Params);
			}
		}
	}

	Global.IncludeNestingLevel--;
	fclose(Input);
	free(Text);
}

bool ConditionalAssemblyTrue()
{
	if (Global.ConditionData.EntryCount != 0)
	{
		for (int i = 0; i < Global.ConditionData.EntryCount; i++)
		{
			if (Global.ConditionData.Entries[i].ConditionTrue == false) return false;
		}
	}
	return true;
}

bool EncodeAssembly()
{
	bool Revalidate;
	Arch->Pass2();
	int validationPasses = 0;
	do	// die schleife so lange durchlaufen lassen bis alles konstant ist
	{
		Global.validationPasses = validationPasses;
		Global.ErrorQueue.Clear();
		Revalidate = false;

		if (validationPasses >= 100)
		{
			QueueError(ERROR_ERROR,"Stuck in infinite validation loop");
			break;
		}

		Global.RamPos = 0;
		Arch->Revalidate();

#ifdef _DEBUG
		printf("Validate %d...\n",validationPasses);
#endif

		for (size_t i = 0; i < Global.Commands.size(); i++)
		{
			if (Global.Commands[i]->IsConditional() == false)
			{
				if (ConditionalAssemblyTrue() == false) continue;
			}

			Global.Commands[i]->SetFileInfo();

			for (int l = 0; l < Global.AreaData.EntryCount; l++)
			{
				if (Global.AreaData.Entries[l].MaxRamPos < Global.RamPos)
				{
					QueueError(ERROR_ERROR,"Area at %s(%d) overflown",
						Global.FileInfo.FileList.GetEntry(Global.AreaData.Entries[l].FileNum),
						Global.AreaData.Entries[l].LineNumber);
				}
			}

			if (Global.Commands[i]->Validate() == true)
			{
				Revalidate = true;
			}
		}
		if (Global.ConditionData.EntryCount != 0) QueueError(ERROR_ERROR,"One or more if statements not terminated");
		validationPasses++;
	} while (Revalidate == true);

	Global.ErrorQueue.Output();

	if (Global.Error == true)
	{
		return false;
	}

	WriteTempFile();
	if (Global.SymData.Write == true)
	{
		Global.SymData.Handle = fopen(Global.SymData.Name,"w");
		fprintf(Global.SymData.Handle,"00000000 0\n");
	}

#ifdef _DEBUG
	printf("Encode...\n");
#endif

	// und schlieﬂlich enkodieren	
	for (size_t i = 0; i < Global.Commands.size(); i++)
	{
		if (Global.Commands[i]->IsConditional() == false)
		{
			if (Global.ConditionData.EntryCount != 0)
			{
				if (ConditionalAssemblyTrue() == false)
				{
					delete Global.Commands[i];
					continue;
				}
			}
		}

		Global.Commands[i]->SetFileInfo();
		Global.Commands[i]->Encode();
		delete Global.Commands[i];
	}

	if (Global.SymData.Write == true)
	{
		fputc(0x1A,Global.SymData.Handle);	// eof zeichen
		fclose(Global.SymData.Handle);
	}

	if (Global.Output.isOpen() == true)
	{
		PrintError(ERROR_WARNING,"File not closed");
		Global.Output.close();
	}

	return true;
}
