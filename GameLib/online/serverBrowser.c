class Room: Managed
{
	private void Room();
	private void ~Room();
	
	/**
	\brief Returns true for official dedicated server and false for community servers.
	*/
	proto native bool Official();
	/**
	\brief Does the server require a password to join.
	*/
	proto native bool PasswordProtected();
	/**
	\brief Was the password verified and may the client join the server or view its content.
	*/
	proto native bool IsAuthorized();
	/**
	\brief Returns false if the server is not ready to receive players (e.g. is restarting / has some network issues / just crashed etc.)
	*/
	proto native bool Joinable();

	proto native bool IsModded();
	proto native bool IsFavorite();
	proto native int PlayerLimit();
	proto native int PlayerCount();
	proto native string GameVersion();
	proto native string Region();
	proto native string Name();
	proto native string ScenarioName();
	
	/**
	\brief Returns maximum size of the join queue.
	*/
	proto native int GetQueueMaxSize();
	
	/**
	\brief Returns current size of the join queue.
	*/
	proto native int GetQueueSize();
	
	/**
	\brief Returns current position of user in join queue.
	\note  Returns -1 if user is not in queue of this room. Must be called on room provided by GetJoinRoom()
	*/
	proto native int GetQueueUserPosition();
	/**
	\brief Returns average time (in seconds) to wait one position in this queue.
	\note  Returns -1 if there wasn't enough data to compute this value.
	\note  Value calculation is based on how long in average it took previous people in queue to join the server.
	*/
	proto native int GetQueueAvgWaitTime();
	/**
	\brief Returns time (in seconds) of how long you are in queue since joining it.
	\note  Returns -1 if you are not in queue.
	*/
	proto native int GetQueueJoinTime();
	/**
	\brief Returns time (in seconds) of how long ago you last moved in queue.
	\note  Returns -1 if you did not yet moved in queue.
	*/
	proto native int GetQueueLastMoveTime();
	/**
	\brief Set BackendCallback which will be used in periodical polls in queue
	*/
	proto native void SetQueueBackendCallback(BackendCallback callback);
	/**
	\brief Leave join queue
	*/
	proto native void LeaveJoinQueue();

	/**
	\brief Addon that contains the hosted scenario
	*/
	proto native Dependency HostScenarioMod();
	proto native string HostAddress();
	/**
	\brief Specifies type of server. Check backend documentation for specific values.
	*/
	proto native string HostType();
	/**
	\brief BI-account name of the server owner (if known)
	*/
	proto native string OwnerName();
	proto native string GameMode();
	
	proto native void Join(BackendCallback callback, JsonApiStruct params);
	
	/**
	\brief Get the list of addons that are present on the server. Available just if IsAuthorized() returns true
	*/
	proto native int GetItems(out notnull array<Dependency> items);
	proto native void SetFavorite(bool favorite, BackendCallback callback);
	
	/**
	\brief Are client's mods the same as in this room?
	*/
	proto native bool IsClientReady();
	
	proto native bool IsCrossPlatform();
	
	proto native MissionWorkshopItem HostScenario();
	
	/**
	\brief Load informations about hosted addons. User has to be authorized first. 
	*/
	proto native void LoadDownloadList(BackendCallback callback);
	
	/**
	\brief Get the last measured ping value. The ping is measured between client and a ping site that corresponds to the server
	*/
	proto native float GetPing();
	
	proto native bool HasBattlEye();
	
	/**
	\brief Check if the server is matching
	*/
	proto native void VerifyPassword(string password, BackendCallback callback);
	
	/**
	\brief Check on reconnect for password protected server 
	- player should be able to rejoin without reentering the password
	*/
	proto native void CheckAuthorization(BackendCallback callback);
	
	/**
	\brief Is the list of hosted addons ready or should we load it (LoadDownloadList)
	*/
	proto native bool IsDownloadListLoaded();
}

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
	
	/**
	\brief The total room count by the current filter.
	*/
	proto native int TotalRoomCount();
	
	//only debug api
	proto native void KillGeneratedRooms();
	proto native void GenerateRooms();
	//only debug api

	/**
	\brief Get room for the room player is currently joining to or playing on.
	\note Returns null if no room is available
	*/
	proto native Room GetJoinRoom();

	/**
	\brief Get room for the last invite request. Returns null if no room is available
	*/
	proto native Room GetInviteRoom();
	/**
	\brief Clear the room from GetInviteRoom()
	*/
	proto native bool InvitationFailed();
	/**
	\brief Clear the room from GetInviteRoom()
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
	
	/**
	\brief Set callback for periodical room refresh
	*/
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
	
	/**
	\brief Get automatically detected IP of the network adapter.
	TODO: This will be obsolete when the proper LAN server support is ready
	*/
	proto native string GetMyIP();
	
	/**
	\brief In case of a disconnection you can retrieve the previous room ID.
		It should be cleared after that to prevent acquiring the same id twice.
	*/
	proto native string GetPreviousRoomId();
	proto native void ClearPreviousRoomId();
	
	/**
	\brief Find the room by its ID in cached data
	*/
	proto native Room GetRoomById(string id);
	
	/**
	\brief Retrieve the pings sites and start to measure latency. 
	*/
	proto native void MeasureLatency(BackendCallback callback);
	
	/**
	\brief True if ping data to the ping sites are ready to use
	*/
	proto native bool IsPingAvailable();
	
	/**
	\brief Triggers OnError/OnTimeout in case the previous listen server initialization failed.
	*/
	proto native void ProcessLastHostError(BackendCallback callback);
}

