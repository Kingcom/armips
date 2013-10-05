#include "StdAfx.h"
#include "CMacro.h"

void CMacro::LoadArguments(CArgumentList& ArgumentList)
{
	strcpy(Name,ArgumentList.GetEntry(0));
	for (int i = 1; i < ArgumentList.GetCount(); i++)
	{
		Arguments.AddEntry(ArgumentList.GetEntry(i));
	}
}


void CMacro::GetLine(int num, CArgumentList& ArgumentValues, char* dest, int MacroCounter)
{
	char* Source = Lines.GetEntry(num);
	char Buffer[128];
	char StringBuffer[512];
	int StringBufferPos = 0;

	// parameter einsetzen
	while (*Source != 0)
	{
		while ((!(*Source >= 'A' && *Source <= 'Z'))
			&& (!(*Source >= 'a' && *Source <= 'z')))
		{
			if (*Source == 0) break;
			StringBuffer[StringBufferPos++] = *Source++;
		}
		if (*Source == 0) break;
		int k = 0;
		while ((*Source >= 'A' && *Source <= 'Z')
			|| (*Source >= 'a' && *Source <= 'z')
			|| (*Source >= '0' && *Source <= '9'))
		{
			if (*Source == 0) break;
			Buffer[k++] = *Source++;
		}
		Buffer[k] = 0;

		bool insert = false;
		for (int i = 0; i < Arguments.GetCount(); i++)
		{
			if (strcmp(Buffer,Arguments.GetEntry(i)) == 0)
			{
				StringBufferPos += sprintf(&StringBuffer[StringBufferPos],"%s",ArgumentValues.GetEntry(i));
				insert = true;
				break;
			}
		}
		if (insert == false)
		{
			StringBufferPos += sprintf(&StringBuffer[StringBufferPos],"%s",Buffer);
		}
	}
	StringBuffer[StringBufferPos] = 0;

	// labels ersetzen
	StringBufferPos = 0;
	while (StringBuffer[StringBufferPos] != 0)
	{
		// label
		if (StringBuffer[StringBufferPos+0] == '@' && StringBuffer[StringBufferPos+1] == '@')
		{
			dest += sprintf(dest,"@@%s_%08X_",Name,MacroCounter);
			StringBufferPos += 2;
			continue;
		}
		*dest++ = StringBuffer[StringBufferPos++];
	}
	*dest = 0;
}
