#include "stdafx.h"
#include "Commands/CDirectiveConditional.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Archs/ARM/Arm.h"
#include "Util/Util.h"

extern CArmArchitecture Arm;

CDirectiveConditional::CDirectiveConditional()
{
	Value = 0;
	IsNum = true;
	IsNumSolved = false;
}

bool CDirectiveConditional::Load(CArgumentList& Args, int command)
{
	CStringList List;

	Type = command;

	switch (Type)
	{
	case CONDITIONAL_IF:
	case CONDITIONAL_ELSEIF:
		if (ConvertInfixToPostfix(Args.GetEntry(0),List) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(0));
			return false;
		}
		if (CheckPostfix(List,true) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(0));
			return false;
		}
		Expression.Load(List);
		break;
	case CONDITIONAL_IFDEF:
	case CONDITIONAL_IFNDEF:
	case CONDITIONAL_ELSEIFDEF:
	case CONDITIONAL_ELSEIFNDEF:
		if (Global.symbolTable.isValidSymbolName(convertUtf8ToWString(Args.GetEntry(0))) == false)
		{
			PrintError(ERROR_ERROR,"Invalid label name \"%s\"",Args.GetEntry(0));
			return false;
		}
		strcpy(LabelName,Args.GetEntry(0));
		break;
	case CONDITIONAL_IFARM:
	case CONDITIONAL_IFTHUMB:
		Value = (Arch == &Arm);
		Value |= (Arm.GetThumbMode() << 1);
		break;
	}
	return true;
}

void CDirectiveConditional::Execute()
{
	bool b;

	switch (Type)
	{
	case CONDITIONAL_IFARM:
		if (Global.ConditionData.EntryCount >= 128)
		{
			QueueError(ERROR_ERROR,"Maximum nesting level reached");
			return;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionTrue =
			Value == 1 ? true : false;
		if (Value == 1) Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = true;
		else Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = false;
		Global.ConditionData.Entries[Global.ConditionData.EntryCount++].ElseCase = false;
		break;
	case CONDITIONAL_IFTHUMB:
		if (Global.ConditionData.EntryCount >= 128)
		{
			QueueError(ERROR_ERROR,"Maximum nesting level reached");
			return;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionTrue =
			Value == 3 ? true : false;
		if (Value == 3) Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = true;
		else Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = false;
		Global.ConditionData.Entries[Global.ConditionData.EntryCount++].ElseCase = false;
		break;
	case CONDITIONAL_IF:
		if (Global.ConditionData.EntryCount >= 128)
		{
			QueueError(ERROR_ERROR,"Maximum nesting level reached");
			return;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionTrue =
			Value != 0 ? true : false;
		if (Value != 0) Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = true;
		else Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = false;
		Global.ConditionData.Entries[Global.ConditionData.EntryCount++].ElseCase = false;
		break;
	case CONDITIONAL_ELSE:
		if (Global.ConditionData.EntryCount == 0)
		{
			QueueError(ERROR_ERROR,"No if clause active");
			return;
		}
		if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet == false)
		{
			if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase == true)
			{
				QueueError(ERROR_ERROR,"Else case already defined");
				return;
			}
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue =
				!Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue;
		} else {
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = false;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase = true;
		break;
	case CONDITIONAL_ELSEIF:
		if (Global.ConditionData.EntryCount == 0)
		{
			QueueError(ERROR_ERROR,"No if clause active");
			return;
		}
		if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet == false)
		{
			if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase == true)
			{
				QueueError(ERROR_ERROR,"Else case already defined");
				return;
			}
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue =
				Value != 0 ? true : false;
			if (Value != 0) Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet = true;
		} else {
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = false;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase = false;
		break;
	case CONDITIONAL_ENDIF:
		if (Global.ConditionData.EntryCount == 0)
		{
			QueueError(ERROR_ERROR,"No if clause active");
			return;
		}
		Global.ConditionData.EntryCount--;
		break;
	case CONDITIONAL_IFDEF:
		b = checkLabelDefined(convertUtf8ToWString(LabelName));
/*		switch (Global.Labels.CheckLabel(LabelName,Global.Section))
		{
		case LABEL_UNDEFINED:
		case LABEL_DOESNOTEXIST:
			b = false;
			break;
		case LABEL_DEFINED:
			b = true;
			break;
		}*/

		if (Global.ConditionData.EntryCount == 128)
		{
			QueueError(ERROR_ERROR,"Maximum nesting level reached");
			return;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionTrue = b;
		if (b == true) Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = true;
		else Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = false;
		Global.ConditionData.Entries[Global.ConditionData.EntryCount++].ElseCase = false;
		break;
	case CONDITIONAL_IFNDEF:
		b = !checkLabelDefined(convertUtf8ToWString(LabelName));
/*		switch (Global.Labels.CheckLabel(LabelName,Global.Section))
		{
		case LABEL_UNDEFINED:
		case LABEL_DOESNOTEXIST:
			b = true;
			break;
		case LABEL_DEFINED:
			b = false;
			break;
		}*/

		if (Global.ConditionData.EntryCount == 128)
		{
			QueueError(ERROR_ERROR,"Maximum nesting level reached");
			return;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionTrue = b;
		if (b == true) Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = true;
		else Global.ConditionData.Entries[Global.ConditionData.EntryCount].ConditionMet = false;
		Global.ConditionData.Entries[Global.ConditionData.EntryCount++].ElseCase = false;
		break;
	case CONDITIONAL_ELSEIFDEF:	
		if (Global.ConditionData.EntryCount == 0)
		{
			QueueError(ERROR_ERROR,"No if clause active");
			return;
		}
		if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet == false)
		{
			if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase == true)
			{
				QueueError(ERROR_ERROR,"Else case already defined");
				return;
			}
			b = checkLabelDefined(convertUtf8ToWString(LabelName));
/*			switch (Global.Labels.CheckLabel(LabelName,Global.Section))
			{
			case LABEL_UNDEFINED:
			case LABEL_DOESNOTEXIST:
				b = false;
				break;
			case LABEL_DEFINED:
				b = true;
				break;
			}*/
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = b;
			if (b == true) Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet = true;
		} else {
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = false;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase = false;
	case CONDITIONAL_ELSEIFNDEF:	
		if (Global.ConditionData.EntryCount == 0)
		{
			QueueError(ERROR_ERROR,"No if clause active");
			return;
		}
		if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet == false)
		{
			if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase == true)
			{
				QueueError(ERROR_ERROR,"Else case already defined");
				return;
			}

			b = !checkLabelDefined(convertUtf8ToWString(LabelName));
/*			switch (Global.Labels.CheckLabel(LabelName,Global.Section))
			{
			case LABEL_UNDEFINED:
			case LABEL_DOESNOTEXIST:
				b = true;
				break;
			case LABEL_DEFINED:
				b = false;
				break;
			}*/
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = b;
			if (b == true) Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionMet = true;
		} else {
			Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue = false;
		}
		Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ElseCase = false;
	}
}

bool CDirectiveConditional::Validate()
{
	bool Result = false;
	int num;

	switch (Type)
	{
	case CONDITIONAL_IF:
	case CONDITIONAL_ELSEIF:
		if (ParsePostfix(Expression,NULL,num) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression");
			return false;
		}

		if (Value != num) Result = true;
		Value = num;
		break;
	}

	Execute();
	return Result;
}

void CDirectiveConditional::WriteTempData(FILE*& Output)
{
	Execute();
}

void CDirectiveConditional::Encode()
{
	Execute();
}
