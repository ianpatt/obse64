#pragma once

#include <cstdint>

typedef std::uint32_t PluginHandle;	// treat this as an opaque type

class BranchTrampoline;

struct PluginInfo
{
	enum
	{
		kInfoVersion = 1
	};

	std::uint32_t	infoVersion;
	const char *	name;
	std::uint32_t	version;
};

enum
{
	kPluginHandle_Invalid = 0xFFFFFFFF
};

enum
{
	kInterface_Invalid = 0,
	kInterface_Messaging,
	kInterface_Trampoline,
	kInterface_Max,
};

struct OBSEInterface
{
	enum
	{
		kInterfaceVersion = 1
	};

	std::uint32_t	obse64Version;
	std::uint32_t	runtimeVersion;
	std::uint32_t	editorVersion;
	std::uint32_t	isEditor;
	std::uint32_t	interfaceVersion;
	void *	(* QueryInterface)(std::uint32_t id);

	// call during your Query or Load functions to get a PluginHandle uniquely identifying your plugin
	// invalid if called at any other time, so call it once and save the result
	PluginHandle		(* GetPluginHandle)(void);

	// returns the OBSE64 build's release index
	std::uint32_t		(* GetReleaseIndex)(void);

	// returns the plugin info structure for a plugin by name, only valid to be called after PostLoad message
	const PluginInfo*	(* GetPluginInfo)(const char * name);

	// returns the folder name the game is using in the My Games folder
	const char*			(* GetSaveFolderName)(void);
};

/**** Messaging API docs ********************************************************************
 *
 *	Messaging API allows inter-plugin communication at run-time. A plugin may register
 *	one callback for each plugin from which it wishes to receive messages, specifying
 *	the sender by name in the call to RegisterListener(). RegisterListener returns false
 *	if the specified plugin is not loaded, true otherwise. Any messages dispatched by
 *	the specified plugin will then be forwarded to the listener as they occur. Passing NULL as 
 *	the sender registers the calling plugin as a listener to every loaded plugin.
 *
 *	Messages may be dispatched via Dispatch() to either a specific listener (specified
 *	by name) or to all listeners (with NULL passed as the receiver). The contents and format of
 *	messageData are left up to the sender, and the receiver is responsible for casting the message
 *	to the expected type. If no plugins are registered as listeners for the sender, 
 *	Dispatch() returns false, otherwise it returns true.
 *
 *	Calling RegisterListener() or Dispatch() during plugin load is not advised as the requested plugin
 *	may not yet be loaded at that point. Instead, if you wish to register as a listener or dispatch a
 *	message immediately after plugin load, use RegisterListener() during load to register to receive
 *	messages from OBSE64 (sender name: "OBSE"). You will then receive a message from OBSE64 once 
 *	all plugins have been loaded, at which point it is safe to establish communications between
 *	plugins.
 *
 *	Some plugin authors may wish to use strings instead of integers to denote message type. In
 *	that case the receiver can pass the address of the string as an integer and require the receiver
 *	to cast it back to a char* on receipt.
 *
 *********************************************************************************************/

struct OBSEMessagingInterface
{
	struct Message {
		const char		* sender;
		std::uint32_t	type;
		std::uint32_t	dataLen;
		void			* data;
	};

	typedef void (* EventCallback)(Message* msg);

	enum {
		kInterfaceVersion = 1
	};

	// OBSE messages
	enum {
		kMessage_PostLoad,		// sent to registered plugins once all plugins have been loaded (no data)
		kMessage_PostPostLoad,	// sent right after kMessage_PostPostLoad to facilitate the correct dispatching/registering of messages/listeners

		kMessage_DataLoaded,	// sent after the data handler has loaded all its forms. call comes from Oblivion thread
	};

	std::uint32_t interfaceVersion;
	bool	(* RegisterListener)(PluginHandle listener, const char* sender, EventCallback handler);
	bool	(* Dispatch)(PluginHandle sender, std::uint32_t messageType, void * data, std::uint32_t dataLen, const char* receiver);
};

struct OBSETrampolineInterface
{
	enum
	{
		kInterfaceVersion = 1
	};

	std::uint32_t interfaceVersion;

	void * (* AllocateFromBranchPool)(PluginHandle plugin, size_t size);
	void * (* AllocateFromLocalPool)(PluginHandle plugin, size_t size);
};

typedef bool (* _OBSEPlugin_Load)(const OBSEInterface * obse);

/**** plugin versioning ********************************************************
 *
 *	OBSE64 does not call any code to determine plugin compatibility. Checks are
 *	made against 
 *	no longer calls any code. Plugins declare their compatibility, and OBSE64
 *	determines whether to load the plugin. Setting this up is simple, just
 *	add something like this to your project:
 *	

extern "C" {
__declspec(dllexport) OBSEPluginVersionData OBSEPlugin_Version =
{
	OBSEPluginVersionData::kVersion,
	
	1,
	"my awesome plugin",
	"my name",

	0,	// not address independent
	0,	// not structure independent
	{ RUNTIME_VERSION_0_411_140, 0 },	// compatible with 0.411.140 and that's it

	0,	// works with any version of the script extender. you probably do not need to put anything here
	0, 0, 0	// set these reserved fields to 0
};
};

 *	
 ******************************************************************************/

struct OBSEPluginVersionData
{
	enum
	{
		kVersion = 1,
	};

	// describe how you find your addresses
	enum
	{
		// signature matching only
		kAddressIndependence_Signatures = 1 << 0,
		
		// Address Library v1 (https://www.nexusmods.com/oblivionremastered/mods/0)
		kAddressIndependence_AddressLibrary = 1 << 1,
	};
	
	// describe your structure compatibility
	enum
	{
		// doesn't use any game structures
		kStructureIndependence_NoStructs = 1 << 0,
		
		// works with the structure layout the game shipped with (0.411.140)
		kStructureIndependence_InitialLayout = 1 << 1,
		
		// additional bits will be added here when breaking changes happen
	};

	std::uint32_t	dataVersion;		// set to kVersion

	std::uint32_t	pluginVersion;		// version number of your plugin
	char			name[256];			// null-terminated ASCII plugin name (please make this recognizable to users)
	char			author[256];		// null-terminated ASCII plugin author name
	
	// version compatibility
	std::uint32_t	addressIndependence;	// bitfield. describe how you find your addresses using the kAddressIndependence_ enums
	std::uint32_t	structureIndependence;	// bitfield. describe how you handle structure layout using the kStructureIndependence_ enums
	std::uint32_t	compatibleVersions[16];	// zero-terminated list of RUNTIME_VERSION_ defines your plugin is compatible with
	
	std::uint32_t	seVersionRequired;		// minimum version of the script extender required, compared against PACKED_OBSE_VERSION
											// you probably should just set this to 0 unless you know what you are doing
	std::uint32_t	reservedNonBreaking;	// bitfield. set to 0
	std::uint32_t	reservedBreaking;		// bitfield. set to 0
	std::uint8_t	reserved[512];			// set to 0
};

/**** plugin API docs **********************************************************
 *	
 *	The base API is pretty simple. Add version data as shown in the
 *	OBSEPluginVersionData docs above, and export this function:
 *	
 *	bool OBSEPlugin_Load(const OBSEInterface * obse)
 *	
 *	In this function, use the interfaces above to register your commands, patch
 *	memory, generally do whatever you need to for integration with the runtime.
 *	
 *	At this time, or at any point forward you can call the QueryInterface
 *	callback to retrieve an interface structure for the base services provided
 *	by the OBSE64 core.
 *	
 *	Note that all structure versions are backwards-compatible, so you only need
 *	to check against the latest version that you need. New fields will be only
 *	added to the end, and all old fields will remain compatible with their
 *	previous implementations.
 *	
 *	If your plugin needs to make modifications before global initializers, add
 *	and export this:
 *	
 *	bool OBSEPlugin_Preload(const OBSEInterface * obse)
 *	
 *	Game and OBSE64 functionality may be limited during preload.
 *	
 ******************************************************************************/
