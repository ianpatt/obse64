#pragma once

#include "obse64_common/Relocation.h"
#include "obse64_common/Utilities.h"

class TESForm;
class TESObjectREFR;
class TESQuest;

class Script;

struct ScriptLocals;
struct ScriptLineBuffer;
struct ScriptBuffer;

enum ParamType
{
	kParamType_String =				0x00,
	kParamType_Integer =			0x01,
	kParamType_Float =				0x02,
	kParamType_InventoryObject =	0x03,	// GetItemCount				TESForm *, must pass IsInventoryObjectType check
	kParamType_ObjectRef =			0x04,	// Activate					TESObjectREFR *
	kParamType_ActorValue =			0x05,	// ModActorValue			not in switch
	kParamType_Actor =				0x06,	// ToggleAI					TESObjectREFR *, must pass IsActor
	kParamType_SpellItem =			0x07,	// AddSpell					TESForm *, must be either SpellItem or book
	kParamType_Axis =				0x08,	// Rotate					not in switch (X Y Z, passed as char)
	kParamType_Cell =				0x09,	// GetInCell				TESObjectCELL *, must pass sub_4C0780
	kParamType_AnimationGroup =		0x0A,	// PlayGroup				not in switch
	kParamType_MagicItem =			0x0B,	// Cast						MagicItem *
	kParamType_Sound =				0x0C,	// Sound					TESForm *, kFormType_Sound
	kParamType_Topic =				0x0D,	// Say						TESForm *, kFormType_Dialog
	kParamType_Quest =				0x0E,	// ShowQuestVars			TESForm *, kFormType_Quest
	kParamType_Race =				0x0F,	// GetIsRace				TESForm *, kFormType_Race
	kParamType_Class =				0x10,	// GetIsClass				TESForm *, kFormType_Class
	kParamType_Faction =			0x11,	// Faction					TESForm *, kFormType_Faction
	kParamType_Sex =				0x12,	// GetIsSex					not in switch
	kParamType_Global =				0x13,	// GetGlobalValue			TESForm *, kFormType_Global
	kParamType_Furniture =			0x14,	// IsCurrentFurnitureObj	TESForm *, kFormType_Furniture
	kParamType_TESObject =			0x15,	// PlaceAtMe				TESObject *
	kParamType_VariableName =		0x16,	// GetQuestVariable			not in switch
	kParamType_QuestStage =			0x17,	// SetStage					handled like integer
	kParamType_MapMarker =			0x18,	// ShowMap					TESObjectREFR *, base form must be dword_AF36F8
	kParamType_ActorBase =			0x19,	// SetEssential				TESActorBase *
	kParamType_Container =			0x1A,	// RemoveMe					TESObjectREFR *, must pass TESObjectREFR_GetContainer
	kParamType_WorldSpace =			0x1B,	// CenterOnWorld			TESWorldSpace *
	kParamType_CrimeType =			0x1C,	// GetCrimeKnown			not in switch
	kParamType_AIPackage =			0x1D,	// GetIsCurrentPackage		TESPackage *
	kParamType_CombatStyle =		0x1E,	// SetCombatStyle			TESCombatStyle *
	kParamType_MagicEffect =		0x1F,	// HasMagicEffect			EffectSetting *
	kParamType_Birthsign =			0x20,	// GetIsPlayerBirthsign		TESForm *, kFormType_BirthSign
	kParamType_FormType =			0x21,	// GetIsUsedItemType		FormType (not all types supported)
	kParamType_WeatherID =			0x22,	// GetIsCurrentWeather		TESForm *, kFormType_Weather
	kParamType_NPC =				0x23,	// Say						TESForm *, kFormType_NPC
	kParamType_Owner =				0x24,	// IsOwner					TESForm *, kFormType_NPC or kFormType_Faction
	kParamType_EffectShader =		0x25,	// haven't seen used		TESForm *, kFormType_EffectShader

	// custom OBSE types (not reimplemented yet)
	kParamType_StringVar =			0x01,
	kParamType_Array =				0x26,	// only usable with compiler override; StandardCompile() will report unrecognized param type
};

// 10
struct ParamInfo
{
	const char	* typeStr;		// 00
	u32			typeID;			// 08
	bool		isOptional;		// 0C
	u8			pad0D[3];		// 0D
};

static_assert(sizeof(ParamInfo) == 0x10);

typedef bool (* ExecuteFunction)(
	const ParamInfo * paramInfo,
	const char * scriptData,
	TESObjectREFR * thisObj,
	TESObjectREFR * containingObj,
	Script * script,
	ScriptLocals * locals,
	double * result,
	u32 * opcodeOffsetPtr);

typedef bool (* ParseFunction)(
	u32 numParams,
	const ParamInfo * paramInfo,
	ScriptLineBuffer * lineBuf,
	ScriptBuffer * scriptBuf);

typedef bool (* ConditionFunction)(
	TESObjectREFR * thisObj,
	void * arg1,
	void * arg2,
	double * result);

// 50
struct CommandInfo
{
	const char			* name;					// 00
	const char			* shortName;			// 08
	u32					opcode;					// 10
//	u32					pad14;					// 14
	const char			* helpText;				// 18
	bool				needsParent;			// 20
//	u8					pad21;					// 21
	u16					numParams;				// 22
//	u32					pad24;					// 24
	ParamInfo			* params;				// 28

	// handlers
	ExecuteFunction		execute;				// 30
	ParseFunction		parse;					// 38
	ConditionFunction	condition;				// 40

	// flags
	bool				editorFilter;			// 48
	bool				invalidatesCellList;	// 49
//	u8					pad4A[6];				// 4A
};

static_assert(sizeof(CommandInfo) == 0x50);

bool Cmd_Default_Parse(u32 numParams, const ParamInfo * paramInfo, ScriptLineBuffer * lineBuf, ScriptBuffer * scriptBuf);
typedef bool (* _ExtractArgs)(
	const ParamInfo * paramInfo,
	const char * scriptData,
	u32 * opcodeOffsetPtr,
	TESObjectREFR * thisObj,
	TESObjectREFR * containingObj,
	Script * script,
	ScriptLocals * locals,
	...);
extern RelocAddr <_ExtractArgs> ExtractArgs;

// Script values are doubles, and the Remastered keeps a form in one as its form ID's number,
// (double)refID, not as classic Oblivion's raw form ID bits: the game's own commands store
// their results that way (RVA 0x06630DD0), and ref variables are converted back from the number.
inline void SetFormResult(double * result, u32 refID)
{
	*result = double(refID);
}

inline u32 FormIDFromValue(double value)
{
	return u32(s64(value));
}

#define COMMAND_ARGS		const ParamInfo * paramInfo, const char * scriptData, TESObjectREFR * thisObj, TESObjectREFR * containingObj, Script * script, ScriptLocals * locals, double * result, u32 * opcodeOffsetPtr
#define PASS_COMMAND_ARGS	paramInfo, scriptData, thisObj, containingObj, script, locals, result, opcodeOffsetPtr
#define EXTRACT_ARGS		paramInfo, scriptData, opcodeOffsetPtr, thisObj, containingObj, script, locals
#define COMMAND_ARGS_EVAL	TESObjectREFR * thisObj, void * arg1, void * arg2, double * result

enum
{
	kScript_NumScriptCommands = 0x0172,
	kScript_NumConsoleCommands = 0x0086,

	kScript_ScriptOpBase = 0x1000,
	kScript_ConsoleOpBase = 0x0100,

	kScript_OBSEOpBase = 0x1400,
};

extern RelocPtr <CommandInfo>	g_firstScriptCommand;
extern RelocPtr <CommandInfo>	g_firstConsoleCommand;
