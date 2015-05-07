#include "stdafx.h"
#include "Core/Directives.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CDirectiveArea.h"
#include "Commands/CAssemblerLabel.h"
#include "Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/ARM/Arm.h"
#include "Core/Expression.h"
#include "Util/Util.h"

/*bool DirectiveInclude(ArgumentList& List, int flags)
{
	std::wstring fileName = getFullPathName(List[0].text);

	TextFile::Encoding encoding = TextFile::GUESS;
	if (List.size() == 2)
		encoding = getEncodingFromString(List[1].text);

	int FileNum = Global.FileInfo.FileNum;
	int LineNum = Global.FileInfo.LineNumber;
	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::Error,L"Included file \"%s\" does not exist",fileName);
		return false;
	}
	LoadAssemblyFile(fileName,encoding);
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = LineNum;
	return true;
}*/

