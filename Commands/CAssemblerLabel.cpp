#include "Commands/CAssemblerLabel.h"

#include "Archs/ARM/Arm.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"
#include "Util/Util.h"

CAssemblerLabel::CAssemblerLabel(const Identifier& name, const Identifier& originalName)
{
	this->defined = false;
	this->label = nullptr;
	
	if (!Global.symbolTable.isLocalSymbol(name))	
		updateSection(++Global.Section);

	label = Global.symbolTable.getLabel(name, FileNum, getSection());
	if (label == nullptr)
	{
		Logger::printError(Logger::Error, "Invalid label name \"%s\"", name);
		return;
	}

	label->setOriginalName(originalName);

	// does this need to be in validate?
	if (label->getUpdateInfo())
	{
		if (&Architecture::current() == &Arm && Arm.GetThumbMode())
			label->setInfo(1);
		else
			label->setInfo(0);
	}
}

CAssemblerLabel::CAssemblerLabel(const Identifier& name, const Identifier& originalName, Expression& value)
	: CAssemblerLabel(name,originalName)
{
	labelValue = value;
}

bool CAssemblerLabel::Validate(const ValidateState &state)
{
	bool result = false;
	if (!defined)
	{
		if (label->isDefined())
		{
			Logger::queueError(Logger::Error, "Label \"%s\" already defined", label->getName());
			return false;
		}
		
		label->setDefined(true);
		defined = true;
		result = true;
	}
	
	bool hasPhysicalValue = false;
	int64_t virtualValue = 0;
	int64_t physicalValue = 0;

	if (labelValue.isLoaded())
	{
		// label value is given by expression
		if (!labelValue.evaluateInteger(virtualValue))
		{
			Logger::printError(Logger::Error, "Invalid expression");
			return result;
		}
	} else {
		// label value is given by current address
		virtualValue = g_fileManager->getVirtualAddress();
		physicalValue = g_fileManager->getPhysicalAddress();
		hasPhysicalValue = true;
	}

	if (label->getValue() != virtualValue)
	{
		label->setValue(virtualValue);
		result = true;
	}

	if (hasPhysicalValue && (!label->hasPhysicalValue() || physicalValue != label->getPhysicalValue()))
	{
		label->setPhysicalValue(physicalValue);
		result = true;
	}

	return result;
}

void CAssemblerLabel::Encode() const
{

}

void CAssemblerLabel::writeTempData(TempData& tempData) const
{
	if (!Global.symbolTable.isGeneratedLabel(label->getName()))
		tempData.writeLine(label->getValue(),tfm::format("%s:",label->getName()));
}

void CAssemblerLabel::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (label->getValue() == -1 || Global.symbolTable.isGeneratedLabel(label->getName()))
		return;

	symData.addLabel(label->getValue(),label->getOriginalName().string());
}




CDirectiveFunction::CDirectiveFunction(const Identifier& name, const Identifier& originalName)
{
	this->label = std::make_unique<CAssemblerLabel>(name,originalName);
	this->content = nullptr;
	this->start = this->end = 0;
}

bool CDirectiveFunction::Validate(const ValidateState &state)
{
	start = g_fileManager->getVirtualAddress();

	label->applyFileInfo();
	bool result = label->Validate(state);

	ValidateState contentValidation = state;
	contentValidation.noFileChange = true;
	contentValidation.noFileChangeDirective = "function";
	content->applyFileInfo();
	if (content->Validate(contentValidation))
		result = true;

	end = g_fileManager->getVirtualAddress();
	return result;
}

void CDirectiveFunction::Encode() const
{
	label->Encode();
	content->Encode();
}

void CDirectiveFunction::writeTempData(TempData& tempData) const
{
	label->writeTempData(tempData);
	content->applyFileInfo();
	content->writeTempData(tempData);
}

void CDirectiveFunction::writeSymData(SymbolData& symData) const
{
	symData.startFunction(start);
	label->writeSymData(symData);
	content->writeSymData(symData);
	symData.endFunction(end);
}
