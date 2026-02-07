//! enums and base classes for filtering in server browser
//! for specific usage of enum types please check the backend documentation

enum OnlineError
{
	NONE,
	BAD_PARAM,
	CONNECTION,
	SERVER,
	INTERNAL,
}

enum SBPlatform
{
	ANY,
	PC,
	XBL,
	PSN,
}

enum SBRegion
{
	ANY,
	AMERICAS,
	EUROPE,
	ASIA,
}

//flags can be combined
enum SBServerFlag
{
	MODDED = 0x1,
	ANTI_CHEAT = 0x2,
	PRIVATE = 0x4,
	JOINABLE = 0x8,
	PASSWORD = 0x10,
	TEST = 0x20,
	INVISIBLE = 0x40,
}

enum SBSortBy
{
	IDENTIFIER,
	GAME_TYPE,
	GAME_VERSION,
	CLIENT_VERSION,
	ANTI_CHEAT,
	PLATFORM,
	GAME_MODE,
	REGION,
	PASSWORD,
	JOINABLE,
	PRIVATE,
	TEXT,
	MODDED,
	MAX_PLAYERS,
	FREE_SLOTS,
	NAME,
}
enum SBSortOrder
{
	ASCENDING,
	DESCENDING,
}

enum SBServerFilter
{
	NAME = 0x1,
	DESCRIPTION = 0x2,
	GAME_TYPE = 0x4,
	GAME_MODE = 0x8,
	GAME_VERSION = 0x10,
	CLIENT_VERSION = 0x20,
	MAX_PLAYERS = 0x40,
	MIN_FREE_SLOTS = 0x80,
	MODDED = 0x100,
	ANTI_CHEAT = 0x200,
	PRIVATE = 0x400,
	JOINABLE = 0x800,
	PASSWORD = 0x1000,
	TEST = 0x2000,
	INVISIBLE = 0x4000,
}


class SBGetServersParams
{
	int					RowsPerPage;
	int					Page;
	SBSortBy			SortBy;
	SBSortOrder			SortOrder;
	SBPlatform			Platform;
	SBRegion			Region;
	SBServerFilter		Filters;
	SBServerFlag		FilterFlags;
	string				FilterName;
	string				FilterDescription;
	int					FilterGameType;
	int					FilterGameMode;
	string				FilterGameVersion;
	string				FilterClientVersion;
	int					FilterMaxPlayers;
	int					FilterMinFreeSlots;
}

class GetRoomsIds extends JsonApiStruct
{
	ref array<string> ids;
	
	void GetRoomsIds()
	{
		ids = new array<string>;
		RegV("ids");
	}
}

class RoomFilterBase : JsonApiStruct
{
	bool includePing = false;
	bool ownedOnly = false;
}