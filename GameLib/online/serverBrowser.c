enum OnlineError
{
	NONE,
	BAD_PARAM,
	CONNECTION,
	SERVER,
	INTERNAL,
};

enum SBPlatform
{
	ANY,
	PC,
	XBL,
	PSN,
};

enum SBRegion
{
	ANY,
	AMERICAS,
	EUROPE,
	ASIA,
};
	
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
};

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
};

enum SBSortOrder
{
	ASCENDING,
	DESCENDING,
};

//flags can be combined, filters enable specific SBServerFlag values or filters
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
};

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
};

class SBServerInfo
{
	string			Identifier;
	SBServerFlag	Flags;
	string			Name;
	string			Description;
	int				HostPort;
	string			HostIp;
	int				GameType;
	string			GameVersion;
	int				GameMode;
	string			ClientVersion;
	SBRegion		Region;
	SBPlatform		Platform;
	int				MaxPlayers;
	int				FreeSlots;
	int				NumPlayers;
	string			JsonMetadata;
};

// class SBServerParams
// {
// 	SBServerFlag		Flags;
// 	string				Name;
// 	string				Description;
// 	string				ClientVersion;
// 	int					GameType;
// 	string				GameVersion;
// 	int					GameMode;
// 	int					HostPort;
// 	string				HostIp;
// 	SBRegion			Region; //cannot be ANY
// 	SBPlatform			Platform; //cannot be ANY
// 	int					MaxPlayers;
// 	string				JsonMetadata; //set to empty string, not sure if it is working properly
// };

typedef array<ref SBServerInfo> SBServerInfoList;

//callback interface, must be kept alive for the duration of the request
class SBServerListCallback
{
	void OnServerList(
		SBServerInfoList server_info_list,
		int server_count, 
		int page_count, 
		int page_index,
		OnlineError error)
	{	
		//override and implement
	}
}

// //callback interface, must be kept alive until OnServerShutdown is called
// class SBServerCallbacks
// {
// 	void OnServerConnected() { /*override and implement*/ }
// 	void OnServerDisconnected(OnlineError error) { /*override and implement*/ }
// 	void OnServerConnectFailed(OnlineError error) { /*override and implement*/ }
// 	void OnServerShutdown(OnlineError error) { /*override and implement*/ }
// }

// class SBServer
// {
// 	//tasync request, OnServerShutdown will be called, after that the SBServer deletes itself
// 	proto native void Shutdown();
// 	//call this at regular intervals
// 	proto native void Update();
// }


// -------------------------------------------------------------------------
//used only for demonstration
class ServerSearchParams extends JsonApiStruct
{
	string order;
	string text;
	bool ascendent;

	void ServerSearchParams()
	{
		RegV("order");
	}
	
	void SetTestValues()
	{
		order = "SessionName";
		ascendent = false;		
	}
	
	override void OnPack()
	{
		if (text && !text.IsEmpty())
			StoreString("text", text);
		StoreBoolean("ascendent", ascendent);
	}

};

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
}

class Room: Managed
{
	private void Room();
	private void ~Room();
	
	proto native bool Official();
	proto native bool PasswordProtected();
	proto native bool Joinable();
	proto native bool IsFavorite();
	proto native int PlayerLimit();
	proto native int PlayerCount();
	proto native string GameVersion();
	proto native string Region();
	proto native string Name();
	proto native string ScenarioName();
	proto native Dependency HostScenarioMod();
	proto native string HostAddress();
	proto native string HostType();
	proto native string OwnerName();
	proto native string GameMode();
	
	proto native void Join(BackendCallback callback, JsonApiStruct params);
	
	proto native int GetItems(out notnull array<Dependency> items);
	proto native void SetFavorite(bool favorite, BackendCallback callback);
	
	/**
	\brief Are client's mods the same as in this room?
	*/
	proto native bool IsClientReady();
	
	proto native bool IsCrossPlatform();
	
	/**
	\brief Tries to enable all room addons. If it can't enable any items, it just returns problematic items 
	*/
	proto native void EnableRoomAddons(out notnull array<Dependency> errorItems);
	
	
	proto native MissionWorkshopItem HostScenario();
	
	proto native void LoadDownloadList(BackendCallback callback);
	
	proto native void DownloadAddons(BackendCallback callback);
	
	proto native void AllItems(out notnull array<Dependency> items);
	
	proto native float GetPing();
	
	proto native bool Connect();
	
	proto native bool HasBattlEye();
};

class ClientLobbyApi
{
	private void ClientLobbyApi();
	private void ~ClientLobbyApi();
	/**
	\brief Search first batch of rooms.
	*/
	proto native void SearchRooms(JsonApiStruct searchParams, BackendCallback callback);
	/**
	\brief Get room data for current position.
	*/
	proto native int Rooms(out notnull array<Room> rooms);
	proto native void GetRoomsByIds(GetRoomsIds searchParams, BackendCallback callback);
	proto native void GetRoomsByHostIds(GetRoomsIds searchParams, BackendCallback callback);
	/**
	\brief Update current position.
	*/
	proto native void Scroll(int position, BackendCallback callback);
	/**
	\brief View size is expected size of rooms from Rooms(rooms).
	*/
	proto native void SetViewSize(int size);

	proto native int TotalRoomCount();
	
	//only debug api
	proto native void KillGeneratedRooms();
	proto native void GenerateRooms();
	
	proto native Room GetInviteRoom();
	
	proto native bool InvitationFailed();
	/**
	\brief Deallocate scheduled room;
	*/
	proto native void ClearInviteRoom();
	
	/**
	\brief Get parameters created in the current run.
	*/
	proto native JsonApiStruct GetParameters(); 
	
	/**
	\brief Clear serialized data
	*/
	proto native void ClearParams();
	
	/**
	\brief Get serialized parameters
	*/
	proto native string GetStrParams();
	/**
	\brief Serialize params before reload
	*/
	proto native void StoreParams();
	
	proto native void SetRefreshCallback(BackendCallback callback);
	
	/**
	\brief Set a time span between page updates (in ms).
	*/
	proto native void SetRefreshRate(int iRate);
	
	/**
	\brief Get room searched by SearchTarget
	*/
	proto native int Target(out notnull array<Room> rooms);
	
	/**
	\brief Search for a specific room and keep cached data
	*/
	proto native int SearchTarget(JsonApiStruct searchParams, BackendCallback callback);
	
	proto native string GetMyIP();
	
	proto native string GetPreviousRoomId();
	proto native void ClearPreviousRoomId();
	
	proto native Room GetRoomById(string id);
	
	proto native void MeasureLatency(BackendCallback callback);
	proto native bool IsPingAvailable();
};

class ServerBrowser
{
	//does not take ownership of params, callback must be kept alive. Only 1 request at a time.
	proto native static bool 		GetServers(SBGetServersParams params, SBServerListCallback callback);
	// //does not take ownership of params, callbacks must be kept alive, can return NULL on fail
	// proto native static SBServer	StartServer(SBServerParams params, SBServerCallbacks callbacks);
};



