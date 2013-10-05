#include "StdAfx.h"
#include "CLabels.h"

CLabels::CLabels(void)
{
	Labels = (tLabel*) calloc(256,sizeof(tLabel));
	LabelCount = 0;
	LabelsAllocated = 256;

	Equations = (tEquation*) calloc(128,sizeof(tEquation));
	EquationCount = 0;
	EquationsAllocated = 128;

	Data  = (char*) malloc(8192);
	DataPosition = 0;
	DataAllocated = 8192;

	memset(FirstNums,0xFF,sizeof(int)*128);
	memset(LastNums,0xFF,sizeof(int)*128);
}

CLabels::~CLabels(void)
{
	free(Equations);
	free(Labels);
	free(Data);
}

int CLabels::AddText(char* str)
{
	int len = strlen(str)+1;
	if ((DataPosition+len) > DataAllocated)
	{
		DataAllocated = DataAllocated << 1;
		Data = (char*) realloc(Data,DataAllocated);
	}
	int ret = DataPosition;
	memcpy(&Data[DataPosition],str,len);
	DataPosition += len;
	return ret;
}

int CLabels::FindLabel(char* name, int Section, int File)
{
	int num = FirstNums[*name];

	while (num != -1)
	{
		if (Labels[num].Section == Section && Labels[num].File  == File
			&& strcmp(name,GetPointer(Labels[num].NamePointer)) == 0) return num;
		num = Labels[num].Next;
	}
	return -1;
}

int CLabels::GetLabelNum(char* name, int Section, int File)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	return FindLabel(name,Section,File);
}

const char ValidLabelCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

bool CLabels::IsValidName(char* str)
{
	if (IsLocal(str) == true) str += 2;
	if (IsStatic(str) == true) str += 1;

	if (str[0] >= '0' && str[0] <= '9') return false;
	while (*str != 0)
	{
		if (strchr(ValidLabelCharacters,*str++) == NULL) return false;
	}
	return true;
}

int CLabels::AddLabel(char* name, unsigned int value, int Section, int File, bool Defined)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	int num = FindLabel(name,Section,File);
	if (num != -1)	// label schon vorhanden
	{
		if (Defined == false) return LABEL_OK;	// label vorhanden - nicht neu definieren
		if (Labels[num].Defined == true) return LABEL_ALREADYDEFINED;	// label schon vorhanden

		Labels[num].Defined = true;
		Labels[num].Value = value;
		return LABEL_OK;
	}

	if (IsValidName(name) == false) return LABEL_INVALIDNAME;	// kein gültiger name

	if (LabelCount == LabelsAllocated)
	{
		LabelsAllocated = LabelsAllocated << 1;
		Labels = (tLabel*) realloc(Labels,LabelsAllocated*sizeof(tLabel));
	}

	Labels[LabelCount].Value = value;
	Labels[LabelCount].NamePointer = AddText(name);
	Labels[LabelCount].Defined = Defined;
	Labels[LabelCount].Section = Section;
	Labels[LabelCount].File = File;
	Labels[LabelCount].Next = -1;

	// prüfen ob der anfangsbuchstabe schon in der liste ist und entsprechend handeln
	if (LastNums[*name] == -1)
	{
		FirstNums[*name] = LabelCount;
	} else {
		Labels[LastNums[*name]].Next = LabelCount;
	}

	LastNums[*name] = LabelCount++;
	return LABEL_OK;
}

void CLabels::UpdateLabelValue(char* name, int Section, int File, unsigned int NewValue)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	int num = FindLabel(name,Section,File);
	if (num == -1)
	{
		//fehler behandeln
		return;
	}
	Labels[num].Value = NewValue;
}

int CLabels::GetValue(char* name, int Section, int File)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	int num = FindLabel(name,Section,File);
	if (num == -1)
	{
		// fehler behandeln
		return -1;
	}
	return Labels[num].Value;
}


int CLabels::GetValue(int num)
{
	return Labels[num].Value;
}

int CLabels::CheckLabel(char* name, int Section, int File)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	if (IsValidName(name) == false) return LABEL_INVALIDNAME;
	int num = FindLabel(name,Section,File);
	if (num == -1) return LABEL_DOESNOTEXIST;
	if (Labels[num].Defined == false) return LABEL_UNDEFINED;
	else return LABEL_DEFINED;
}

int CLabels::CheckLabel(int num)
{
	if (num == -1) return LABEL_DOESNOTEXIST;
	if (Labels[num].Defined == false) return LABEL_UNDEFINED;
	else return LABEL_DEFINED;
}

void CLabels::WriteSym(char* name)
{
	FILE* Sym = fopen(name,"w");
	fprintf(Sym,"00000000 0\n");

	for (int i = 0; i< LabelCount; i++)
	{
		fprintf(Sym,"%08X %s\n",Labels[i].Value,GetPointer(Labels[i].NamePointer));
	}
	fputc(0x1A,Sym);	// eof zeichen
	fclose(Sym);
}

bool CLabels::EquationExists(char* name, int Section, int File)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	for (int i = 0; i < EquationCount; i++)
	{
		if (Equations[i].Section == Section && Equations[i].File == File &&
			strcmp(name,GetPointer(Equations[i].NamePointer)) == 0) return true;
	}
	return false;
}

void CLabels::AddEquation(char* name, char* string, int Section, int File)
{
	if (IsLocal(name) == false) Section = -1;
	if (IsStatic(name) == false) File = -1;

	if (EquationCount == EquationsAllocated)
	{
		EquationsAllocated = EquationsAllocated << 1;
		Equations = (tEquation*) realloc(Equations,EquationsAllocated*sizeof(tEquation));
	}

	Equations[EquationCount].NamePointer = AddText(name);
	Equations[EquationCount].StringPointer = AddText(string);
	Equations[EquationCount].File = File;
	Equations[EquationCount++].Section = Section;
}

void CLabels::InsertEquations(char* str, int Section, int File)
{
	char* OldStr = str;
	char Buffer[2048];
	int BufferPos = 0;

	while (*str != 0)
	{
		for (int i = 0; i < EquationCount; i++)
		{
			if ((Equations[i].Section == -1 || Equations[i].Section == Section) &&
				(Equations[i].File == -1 || Equations[i].File == File))
			{
				char* s = GetPointer(Equations[i].NamePointer);
				int l = 0;
				while (*s != 0)
				{
					if (str[l] == *s)
					{
						l++;
						s++;
						continue;
					}
					break;
				}
				if (*s != 0) continue;
				if ((str[l] >= 'a' && str[l] <= 'z') || (str[l] <= '9' && str[l] >= '0')) continue;
				strcpy(&Buffer[BufferPos],GetPointer(Equations[i].StringPointer));
				BufferPos += strlen(GetPointer(Equations[i].StringPointer));
				str += l;
				break;
			}
		}
		if (*str == 0) break;
		Buffer[BufferPos++] = *str++;
	}
	Buffer[BufferPos] = 0;
	strcpy(OldStr,Buffer);
}
