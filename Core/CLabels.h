#pragma once

// rückgabewerte
#define LABEL_OK				0
#define	LABEL_ALREADYDEFINED	1
#define LABEL_INVALIDNAME		2
#define LABEL_DEFINED			3
#define LABEL_UNDEFINED			4
#define LABEL_DOESNOTEXIST		5

typedef struct {
	int NamePointer;
	unsigned int Value;
	int Section;
	int File;
	int Next;
	bool Defined;
} tLabel;

typedef struct {
	int NamePointer;
	int StringPointer;
	int Section;
	int File;
} tEquation;


class CLabels
{
public:
	CLabels(void);
	~CLabels(void);
	bool IsValidName(char* str);
	bool IsLocal(char* string) { return (string[0] == '@' && string[1] == '@') ? true : false; };
	bool IsStatic(char* string) { return (string[0] == '@' && string[1] != '@') ? true : false; };
	int AddLabel(char* name, unsigned int value, int Section, int File, bool Defined);
	void UpdateLabelValue(char* name, int Section, int File, unsigned int NewValue);
	int GetValue(char* name, int Section, int File);
	int GetValue(int num);
	int CheckLabel(char* name, int Section, int File);
	int CheckLabel(int num);
	int GetLabelNum(char* name, int Section, int File);
	char* GetName (int num) { return GetPointer(Labels[num].NamePointer); };
	void WriteSym(char* name);
	void AddEquation(char* name, char* string, int Section, int File);
	bool EquationExists(char* name, int Section, int File);
	void InsertEquations(char* str, int Section, int File);
	int GetLabelCount() { return LabelCount; };
	int GetEquationCount() { return EquationCount; };
private:
	int FindLabel(char* name, int Section, int File);
	char* GetPointer(int pos) { return &Data[pos]; };
	int AddText(char* str);
	tLabel* Labels;
	int LabelCount;
	int LabelsAllocated;
	tEquation* Equations;
	int EquationCount;
	int EquationsAllocated;
	char* Data;
	int DataPosition;
	int DataAllocated; 
	int FirstNums[128];	// erster label mit diesem anfangsbuchstaben
	int LastNums[128];	// letzter label mit diesem anfangsbuchstaben
};
