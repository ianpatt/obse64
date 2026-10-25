#include "GameScript.h"
#include "ParamInfos.h"
#include "GameObjects.h"
#include "GameRTTI.h"
#include "GameConsole.h"

static bool Cmd_GetParentCell_Execute(COMMAND_ARGS)
{
	*result = 0;

	if(!thisObj) return true;

	auto * currentCell = DYNAMIC_CAST(thisObj->parentCell, TESObjectCELL, TESForm);

	if(currentCell) {
		if(IsConsoleMode()) Console_Print("Cell: %08x", currentCell->refID);
		SetFormResult(result, currentCell->refID);
	}

	return true;
}

CommandInfo kCommandInfo_GetParentCell =
{
	"GetParentCell", "gcel",
	0,
	"returns a ref to the cell the object is in",
	1,
	0, nullptr,
	Cmd_GetParentCell_Execute
};
