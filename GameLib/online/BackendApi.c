
/** @file */


	//! Backend error
	enum EBackendError
	{
		EBERR_OK,					// all OK
		EBERR_UNKNOWN,				// unknown error
		EBERR_DISABLED,				// backend is disabled
		EBERR_INVALID_STATE,		// called request from state where it is not possible (ie. reading data before logon and such)
		EBERR_BUSY,					// no request can be called - login/ auth in process
		EBERR_ALREADY_OFFLINE,		// state is already active
		EBERR_ALREADY_ONLINE,		// state is already active
		EBERR_ALREADY_REQUESTED,	// state already requested once!
		EBERR_LOGIN_FAILED,			// failed to logon
		EBERR_AUTH_FAILED,			// failed to authenticate
		EBERR_LOGIN_SUCCESS,		// logon successfull
		EBERR_AUTH_SUCCESS,			// authenticate successfull
		EBERR_CONFIGURATION_GET,	// configuration received
		EBERR_CONFIGURATION_WRITE,	// configuration written
		EBERR_CHARACTER_GET,		// character data receieved
		EBERR_CHARACTER_UPDATE,		// character update done
		EBERR_FILE_NOT_FOUND,		// save point doesn't exist
		EBERR_UNSUPPORTED_REQUEST,	// non-supported request call performed
		EBERR_STORAGE_IS_FULL, 		// unable to store data
		EBERR_VALIDATION_FAILED		// downloaded asset is invalid
	}
	

	//! Backend request
	enum EBackendRequest
	{
		EBREQ_GAME_Test,
		EBREQ_GAME_CharacterGet,
//		#if BACKENDAPI_DEV_CHARACTER
		EBREQ_GAME_DevCharacterGet,
		EBREQ_GAME_DevCharacterUpdate,
//		#endif

		EBREQ_GAME_CharacterUpdateS2S,
	
			// client lobby
		EBREQ_LOBBY_RoomsJoin,
		EBREQ_LOBBY_RoomsSearch,
		EBREQ_LOBBY_TargetSearch,
		EBREQ_LOBBY_RoomsGetByIds,
		EBREQ_LOBBY_RoomsGetByHostIds,
		EBREQ_LOBBY_AddFavoriteServer,
		EBREQ_LOBBY_RemoveFavoriteServer,
		EBREQ_LOBBY_ClientRoomsRegister,
		EBREQ_LOBBY_RoomsHeartBeat,
		EBREQ_LOBBY_RoomListUpdate,
		EBREQ_LOBBY_UpdateRooms,
		EBREQ_LOBBY_GetPingSites,
		EBREQ_LOBBY_GetInviteRoom,
		EBREQ_LOBBY_CreateServerOwnerToken,
		EBREQ_LOBBY_VerifyPassword,
		// server config
		EBREQ_LOBBY_SaveServerConfig,
		EBREQ_LOBBY_DownloadServerConfig,
		EBREQ_LOBBY_ListServerConfig,
		EBREQ_LOBBY_DeleteServerConfig,

		// workshop
		EBREQ_WORKSHOP_GetAssetList,
		EBREQ_WORKSHOP_CheckAssets,
		EBREQ_WORKSHOP_GetOwnAssetRating,
		EBREQ_WORKSHOP_PutAssetRating,
		EBREQ_WORKSHOP_DeleteAssetRating,
		EBREQ_WORKSHOP_GetDownloadURL,
		EBREQ_WORKSHOP_PostSubscriptions,
		EBREQ_WORKSHOP_DeleteSubscriptions,
		EBREQ_WORKSHOP_GetAsset,
		EBREQ_WORKSHOP_ReportAsset,
		EBREQ_WORKSHOP_DeleteAssetReport,
		EBREQ_WORKSHOP_GetOwnAssetReport,
		EBREQ_WORKSHOP_PostFavourite,
		EBREQ_WORKSHOP_DeleteFavourite,
		EBREQ_WORKSHOP_GetDependencyTree,
		EBREQ_WORKSHOP_GetAssetScenarios,
		EBREQ_WORKSHOP_GetDownloadList,
		EBREQ_WORKSHOP_GetDownloadListS2S,
	
		EBREQ_WORKSHOP_AddProfileBlock,
		EBREQ_WORKSHOP_RemoveProfileBlock,

		EBREQ_WORKSHOP_UploadAsset,
		EBREQ_WORKSHOP_StatusAsset,
		EBREQ_WORKSHOP_PatchAsset,
		EBREQ_WORKSHOP_DeleteAsset,
		
		EBREQ_WORKSHOP_DownloadAsset,
		EBREQ_WORKSHOP_DownloadImage,
		EBREQ_WORKSHOP_ValidateAsset,
	
		EBREQ_WORKSHOP_DownloadFragment,
		EBREQ_WORKSHOP_DirectDownloadFile
	}
	
	//! Credential parameters
	enum EBackendCredentials
	{
		EBCRED_NAME,
		EBCRED_PWD,
		EBCRED_BASEURI,
		EBCRED_PLATFORMUID,
		EBCRED_2FA_TOKEN
	}

	//! State of Dedicated Server box in cloud (regarding Backend connectivity)
	enum EDsBoxState
	{
		EDSSTATUS_VOID,						// not initiated yet, not requested
		EDSSTATUS_REQUESTED,				// request send to cloud, awaiting answer
		EDSSTATUS_REQFAILED,				// request failed
		EDSSTATUS_REQSUCCESS,				// request successful - server active
		EDSSTATUS_AWAITING,					// box awaiting configuration
		EDSSTATUS_CONFIGURE,				// configuration send to cloud, awaiting answer
		EDSSTATUS_CFGFAILED,				// cfg request failed
		EDSSTATUS_CFGSUCCESS,				// cfg request successful - box configured
		EDSSTATUS_LAUNCHING,				// box launching chose scenario (initializing new or loading from save)
		EDSSTATUS_LAUNCHED,					// box launched (game may start)
		EDSSTATUS_GAME,						// box processing game
		EDSSTATUS_IDLE,						// box is in idle mode (this is state where it saves resources - idling without players)
		EDSSTATUS_ERROR,					// box encountered error during game/ idle mode
		EDSSTATUS_RECOVERY,					// box attempting to recovery
		EDSSTATUS_RESTARTING,				// box is restarting
		EDSSTATUS_BCKFAILED,				// backup request failed
		EDSSTATUS_BCKSUCCESS,				// backup request successful - box stored
		EDSSTATUS_SHUTDOWN,					// box is terminating
	}

	//! Session state (game hosted on Dedicated Server)
	enum EDsSessionState
	{
		EDSESSION_OFFLINE,					// session is not initialized
		EDSESSION_LAUNCHING,				// session is handling online services, connections and loading initial data
		EDSESSION_INIT,						// session is waiting for script/ game to finish initialization (load world) and such
		EDSESSION_ACTIVE,					// session is running - players can connect
		EDSESSION_CLOSING,					// session is being terminated
		EDSESSION_PAUSED,					// session is paused (this is state where server was hibernated)
		EDSESSION_RESTARTING,				// session is being restarted
	}

enum EWorkshopItemType
{
	EWTYPE_UNKNOWN,
	EWTYPE_ADDON,
	EWTYPE_WORLD_SAVE
}

// -------------------------------------------------------------------------
// Dedicated Server Box data record
class DSBox
{
	/**
	\brief Name of the the hosted instance
	*/
	proto native string Name();

	/**
	\brief Status of the hosted instance (EDsBoxState)
	*/
	proto native int Status();
	/**
	\brief Request pending upon hosted instance (EDsBoxState)
	only following are possible EDSSTATUS_REQUESTED/ EDSSTATUS_CONFIGURE/ EDSSTATUS_BACKUP/ EDSSTATUS_SHUTDOWN
	if EDSSTATUS_VOID, no request pending ATM
	*/
	proto native int Request();

	/**
	\brief Player limit on the instance
	*/
	proto native int PlayerLimit();
	/**
	\brief Player count on the instance
	*/
	proto native int PlayerCount();

	/**
	\brief Request server to be hosted
	*/
	proto native void Launch();
	/**
	\brief Request server to be terminated
	*/
	proto native void Shutdown();

}


// -------------------------------------------------------------------------
// Callback interface for DS Session - must exist for the duration of request!
class DSSessionCallback : Managed
{

	/**
	\brief Session connect event
	*/
	void OnConnect()
	{
	}

	/**
	\brief Session disconnect event
	*/
	void OnDisconnect()
	{
	}

	/**
	\brief Save event handling
	*/
	void OnSaving( string fileName )
	{
	}

	/**
	\brief Load event handling
	*/
	void OnLoaded( string fileName )
	{
	}

	/**
	\brief Setup event handling
	*/
	void OnSetup( string fileName )
	{
	}

	/**
	\brief Event when timed player saving is about to happen
	\param iPlayerId - Id of Player which will be saved
	*/
	void OnPlayerSaveEvent( int iPlayerId )
	{
	}

	/**
	\brief Load Fail event handling
	*/
	void OnLoadFailed( string fileName )
	{
	}

	/**
	\brief Save Fail event handling
	*/
	void OnSaveFailed( string fileName )
	{
	}
	/**
	\brief Save Success event handling
	*/
	void OnSaveSuccess( string fileName )
	{
	}

	/**
	\brief Delete Fail event handling
	*/
	void OnDeleteFailed( string fileName )
	{
	}
	/**
	\brief Delete Success event handling
	*/
	void OnDeleteSuccess( string fileName )
	{
	}

	/**
	\brief Initialize event - here specify what you want to load before game starts
	*/
	void OnInitialize()
	{
	}

	/**
	\brief Initializing new session
	*/
	void OnNew()
	{
	}

	/**
	\brief Ready event handling - point where session goes to game
	*/
	void OnReady()
	{
	}

}
// -------------------------------------------------------------------------
class RCONCommander: Managed
{	
	void ProcessCommand(string sCommand, int iRequestId)
	{
		Print("RCONCommander.ProcessCommand not implemented!");		
	}
}

// -------------------------------------------------------------------------
// Save & Load handler
class SessionStorage
{
	/**
	\brief Return true if storage is initialized - ready to load/ store data
	*/
	proto native bool Initialized();
	/**
	\brief Clear all scheduled operations
	*/
	proto native void ClearScheduler();
	/**
	\brief Request scheduler On/ Off without being removed from queue
	\param fileName - name of file handle
	\param bEnable - if should run or not (Note: by default it run so you typically pause it first)
	*/
	proto native void EnableScheduler( string fileName, bool bEnable );
	/**
	\brief Request player save
	\param iPlayerId Is Player Id used on player identity
	*/
	proto native void RequestPlayerSave( int iPlayerId );
	/**
	\brief Request periodical processing save of session content
	\param fileName - name of file handle
	\param sec - time in seconds
	*/
	proto native void RequestScheduledSave( string fileName, float sec );
	/**
	\brief Request server to process save of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	proto native void RequestSave( string fileName );
	/**
	\brief Request server to process load of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	proto native void RequestLoad( string fileName );
	/**
	\brief Request local save of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	proto native void LocalSave( string fileName );
	/**
	\brief Request local load of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	proto native void LocalLoad( string fileName );
	/**
	\brief Request local delete of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	proto native void LocalDelete( string fileName );
	/**
	\brief Check if file/ handle with thist name exist - local if you're local - online if you're online
	\param fileName - name of file handle
	*/
	proto native bool CheckFileID( string fileName );
	/**
	\brief Assign callback for handling Save & Load events for specific file handle under session
	\param fileName - name of file handle
	\param sessionCallback - name of file handle
	*/
	proto native void AssignFileIDCallback( string fileName, DSSessionCallback sessionCallback );

	/**
	\brief Process session load - You call this method from OnLoad() event of callback.	
	\param pDataObject Represents "master" object as targer for incoming data
	*/
	proto native void ProcessLoad( JsonApiStruct pDataObject, string fileName );
	/**
	\brief Process session save - You call this method from OnSave() event of callback.	
	\param pDataObject Represents "master" object as source of outcoming data
	*/
	proto native void ProcessSave( JsonApiStruct pDataObject, string fileName );
	/**
	\brief Check if online storage privileges are granted (if not - all is stored locally with session)
	*/
	proto native bool GetOnlineWritePrivilege();
	
	/**
	\brief Get a list of all save files that are ready to load
	*/
	proto native int AvailableSaves(out notnull array<string> aSaves);
}


// -------------------------------------------------------------------------
// Server Session
class DSSession
{
	/**
	\brief Name of the the session
	*/
	proto native string Name();

	/**
	\brief Status of the hosted instance (EDsSessionState)
	*/
	proto native int Status();

	/**
	\brief Player limit on the instance
	*/
	proto native int PlayerLimit();
	/**
	\brief Player count on the instance
	*/
	proto native int PlayerCount();

	/**
	\brief Request termination of server
	*/
	proto native void RequestShutdown();
	/**
	\brief Request server to enter active state
	*/
	proto native void RequestActive();
	/**
	\brief Request finishing of hosted session (game won, draw, canceled)
	*/
	proto native void RequestFinish();
	/**
	\brief Request restart of hosted session (with or without new parameters)
	*/
	proto native void RequestRestart();

	/**
	\brief Get current session uptime in seconds
	*/
	proto native float GetUpTime();

	/**
	\brief ID of Room created on server
	*/
	proto native string RoomID();
	/**
	\brief ID of Scenario hosted on server
	*/
	proto native string ScenarioID();
}


// -------------------------------------------------------------------------
// Callback interface for backend - must exist for the duration of request!
class BackendCallback : Managed
{
	/**
	\brief Request finished with error result
	\param code Error code is type of EBackendError
	*/
	void OnError( int code, int restCode, int apiCode )
	{
//		Print("[BackendCallback] OnError: "+ g_Game.GetBackendApi().GetErrorCode(code));
	}

	/**
	\brief Request finished with success result
	\param code Code is type of EBackendRequest
	*/
	void OnSuccess( int code )
	{
//		Print("[BackendCallback] OnSuccess()");
	}

	/**
	\brief Request not finished due to timeout
	*/
	void OnTimeout()
	{
//		Print("[BackendCallback] OnTimeout");
	}

}


// -------------------------------------------------------------------------
// debug api
class SavePoint : JsonApiStruct
{
	string msg = "You don't know the power of the Dark side!";
	
	void SavePoint()
	{
		RegV("msg");
	}

	void Reset()
	{
		msg = "garbage";
	}

	void OnLoad()
	{
		Print("SavePoint::OnLoad()");
		Print(msg);
	}

}


class SaveTestCallback : DSSessionCallback
{
	BackendApi api;
	protected ref SavePoint savePoint = new SavePoint();

	override void OnInitialize()
	{
		api.GetStorage().RequestLoad("campaign");
	}

	override void OnSaving( string fileName )
	{
		api.GetStorage().ProcessSave(savePoint, fileName);
	}
	
	override void OnLoaded( string fileName )
	{
		savePoint.Reset(); // reset is here for proof of concept
		api.GetStorage().ProcessLoad(savePoint, fileName);
		savePoint.OnLoad(); // print recieved message
	}
	
	override void OnSaveSuccess( string fileName )
	{
		Print("Success");
	}
}


// -------------------------------------------------------------------------
// Service Status Item
class ServiceStatusItem
{
	/**
	\brief Name of service
	*/
	proto native string Name();
	/**
	\brief Status of service
	*/
	proto native string Status();
	/**
	\brief Message realted to service
	*/
	proto native string Message();

}


// -------------------------------------------------------------------------
// News Feed Item
class NewsFeedItem
{
	/**
	\brief Title of news
	*/
	proto native string Title();
	/**
	\brief Text of news
	*/
	proto native string Excerpt();
	/**
	\brief News website URL fragment
	*/
	proto native string Slug();
	/**
	\brief Category type
	*/
	proto native string Category();
	/**
	\brief Full website URL
	*/
	proto native string URL();
	/**
	\brief Date & Time
	*/
	proto native string Date();
	/**
	\brief Path to downloaded image
	*/
	proto native string Path();

}


// -------------------------------------------------------------------------
// Backend API access
class BackendApi
{

	void BackendApi()
	{
	}

	void ~BackendApi()
	{
	}


	/**
	\brief Set session callback to recieve all session related events
	\param callback - check all events and respective description upon object
	*/
	proto native void SetSessionCallback(DSSessionCallback callback);
	
	/**
	\brief Get Workshop Api
	*/
	proto native WorkshopApi GetWorkshop();

	/**
	\brief Get Lobby Api
	*/
	proto native ClientLobbyApi GetClientLobby();

	/**
	\brief Get active count of server instances
	*/
	proto native int GetDSBoxcount();
	/**
	\brief Get DS box instance
	*/
	proto native DSBox GetDSBox( int iIndex );

	/**
	\brief Get Session Storage
	*/
	proto native SessionStorage GetStorage();

	/**
	\brief Get DS server instance
	*/
	proto native DSSession GetDSSession();

	/**
	\brief Return count of services
	*/
	proto native int GetStatusCount();
	/**
	\brief Return specific status object
	*/
	proto native ServiceStatusItem GetStatusItem( int iIndex );
	/**
	\brief Return Main status object
	*/
	proto native ServiceStatusItem GetMainStatus();

	/**
	\brief Return count of News messages
	*/
	proto native int GetNewsCount();
	/**
	\brief Return specific News message
	*/
	proto native NewsFeedItem GetNewsItem( int iIndex );
	/**
	\brief Return count of Notification messages
	*/
	proto native int GetNotifyCount();
	/**
	\brief Return specific Notification message
	*/
	proto native NewsFeedItem GetNotifyItem( int iIndex );
	/**
	\brief Return specific Link by it's name
	*/
	proto native string GetLinkItem( string linkName );


	/**
	\brief Shutdown backend - request processing
	*/
	proto native bool Shutdown();


	/**
	\brief Client is Authenticated - relate requests may procceed
	*/
	proto native bool IsAuthenticated();
	/**
	\brief Client is busy - Authentication in process
	*/
	proto native bool IsAuthInProgress();
	/**
	\brief True if HTTP communication enabled (initialization, runtime or shutdown may be pending at same time)
	*/
	proto native bool IsRunning();
	/**
	\brief True if HTTP communication is being activated (initializing yet)
	*/
	proto native bool IsInitializing();
	/**
	\brief True if HTTP communication active (initialized)
	*/
	proto native bool IsActive();
	/**
	\brief Refresh status - ping services to obtain response time
	*/
	proto native void RefreshCommStatus();
	/**
	\brief Get comm test status, 0==not executed, 1==running, 2==finished, 3==failed
	*/
	proto native int GetCommTestStatus();
	/**
	\brief Read service response time in milliseconds, call RefreshCommStatus() to update results
	*/
	proto native float GetCommResponseTime();
	/**
	\brief Get time in seconds since last successful request (limit is 1hr - does not count more)
	*/
	proto native float GetCommTimeLastSuccess();
	/**
	\brief Get time in seconds since last successful request (limit is 1hr - does not count more)
	*/
	proto native float GetCommTimeLastFail();
	/**
	\brief Unlink the bi-account and clear credentials
	*/
	proto native void Unlink(BackendCallback callback);

	
	//! Error code to string
	string GetErrorCode( int code )
	{
		string result;
	
		if( code == EBackendError.EBERR_OK )
			result = "OK";
		else if( code == EBackendError.EBERR_UNKNOWN )
			result = "Offline";
		else if( code == EBackendError.EBERR_DISABLED )
			result = "Communication Disabled";
		else if( code == EBackendError.EBERR_INVALID_STATE )
			result = "Cannot be called from current state";
		else if( code == EBackendError.EBERR_BUSY )
			result = "Busy processing requests";
		else if( code == EBackendError.EBERR_ALREADY_OFFLINE )
			result = "Already disconnected";
		else if( code == EBackendError.EBERR_ALREADY_ONLINE )
			result = "Already connected";
		else if( code == EBackendError.EBERR_LOGIN_FAILED )
			result = "Failed to logon";
		else if( code == EBackendError.EBERR_AUTH_FAILED )
			result = "Failed to Authenticate";
		else
			result = "*";
			
		return result;
	}

	/**
	\brief Called when platform ready and provided all data necessary to connect online
	*/
	void OnPlatformActive( int code )
	{
		//Print("!!! [Backend] Platform Active");
	}

	/**
	\brief Called when platform is not ready - perhaps user is not signed-in
	*/
	void OnPlatformMissing( int code )
	{
		//Print("!!! [Backend] Platform Missing");
	}

	/**
	\brief Called when initiate cannot be called
	*/
	void OnCannotInitiate( int code )
	{
		//Print("!!! [Backend] Cannot Initiate: "+ GetErrorCode(code));
	}
	
	/**
	\brief Called when shutdown cannot be proceeded
	*/
	void OnCannotShutdown( int code )
	{
		//Print("!!! [Backend] Cannot Shutdown: "+ GetErrorCode(code));
	}

	/**
	\brief Called when authentication failed
	*/
	void OnCannotAuth( int code )
	{
		//Print("!!! [Backend] Cannot Authenticate: "+ GetErrorCode(code));
	}

	/**
	\brief Called when step was successfully proceeded
	*/
	void OnSuccess( string step )
	{
		//Print( "[Backend] Successfully Solicited: " + step );
	}
	
	/**
	\brief Called when step failed
	*/
	void OnFail( string step )
	{
		//Print( "[Backend] Failed to Proceed: " + step );
	}

	[Obsolete("Use GetPlayerIdentityId() instead.")]
	string GetPlayerUID( int iPlayerId ) { return GetPlayerIdentityId(iPlayerId); }

	/**
	\brief Get Player Identity ID by Player ID
	\param iPlayerId Is id of player in session
	*/
	proto native string GetPlayerIdentityId(int iPlayerId);

	/**
	\brief Get Local Identity ID on client
	*/
	proto native string GetLocalIdentityId();

	/*!
	\brief Get Player Platform Kind by Player ID
	\param iPlayerId Is id of player in session
	*/
	proto native PlatformKind GetPlayerPlatformKind(int iPlayerId);

	/*!
	\brief Get Player Platform ID by Player ID
	\param iPlayerId Is id of player in session
	\note Xbox and PS Ids are hashed in SHA-256
	*/
	proto native string GetPlayerPlatformId(int iPlayerId);

	/**
	\brief Return true if local platform data are to be used for authentication/ persistency of client (meaningless on server)
	*/
	proto native bool IsLocalPlatformAssigned();

	/**
	\brief Ask specific request with callback result
	\param request Is type of request, which is EBackendRequest
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	*/
	proto native void Request( int request, BackendCallback cb, JsonApiStruct dataObject );

	/**
	\brief Ask player request with callback result from controller (Lobby)
	\param request Is type of request, which is EBackendRequest
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param iPlayerId Is Player Id used on player identity
	*/
	proto native void PlayerRequest( int request, BackendCallback cb, JsonApiStruct dataObject, int iPlayerId );

	/**
	\brief Expand player data upon defined structure, this is Server-Side only!
	\note Data are available only after player was successfully accepted into Room/ Lobby on server
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param iPlayerId Is Player Id used on player identity
	*/
	proto native void PlayerData( JsonApiStruct dataObject, int iPlayerId );

	/**
	\brief Expand settings data upon defined structure, this is Server-Side only!
	\note Data are available only after game was successfully executed and connected to online services
	\param sFileName Is name of settings file you want to access, remember - it must exist!
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	*/
	proto native void SettingsData( string sFileName, JsonApiStruct dataObject );

	/**
	\brief Send feedback message and/ or script object with whatever data on it (additionally it is possible to handle callback as well)
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param message Is custom
	*/
	proto native void FeedbackMessage( BackendCallback cb, JsonApiStruct dataObject, string message );

	/**
	\brief Send feedback image and/ or script object with whatever data on it (additionally it is possible to handle callback as well)
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param fileName Is path to image you want to send
	*/
	proto native void FeedbackImage( BackendCallback cb, JsonApiStruct dataObject, string fileName );
	
	/**
	\brief Set credentials value per item
	\param item Is type of EBackendCredentials parameter you want to set
	\param str Is value itself
	*/
	proto native void SetCredentialsItem( EBackendCredentials item, string str );

	/**
	\brief Get credentials value per item
	\param item Is type of EBackendCredentials parameter you want to read
	*/
	proto native string GetCredentialsItem( EBackendCredentials item );

	/**
	\brief Invoke credentials update (authenticate with new name+password)
	*/
	proto native void VerifyCredentials(BackendCallback callback, bool storeCredentials);
	
	/**
	\brief The bi-account remains locked for X seconds
	*/
	proto native int RemainingAccountLockedTime();
	
	/**
	\brief Return true if BI Account is linked to local Identity
	*/
	proto native bool IsBIAccountLinked();
	
	/**
	\brief Get target backend environment
	*/
	proto native owned string GetBackendEnv();
	
	
	proto native bool GetRunningDSConfig(DSConfig config);
	proto native bool LoadDSConfig(DSConfig config, string fileName);
	proto native bool SaveDSConfig(DSConfig config, string fileName);
	proto native void SetDefaultIpPort(DSConfig config);
	proto native int GetAvailableConfigs(out notnull array<string> configs);
	proto native int GetAvailableConfigPaths(out notnull array<string> configs);
	proto native ServerConfigApi GetServerConfigApi();

	//! Get Ban Service API
	proto native BanServiceApi GetBanServiceApi();
	
	/**
	\brief Check if player is in list of admins defined in server config
	\param iPlayerId is Id of Player which is being verified
	*/
	proto native bool IsListedServerAdmin( int iPlayerId );

	/**
	\brief Check if player is server owner defined by ownerToken
	\param iPlayerId is Id of Player which is being verified
	*/
	proto native bool IsServerOwner( int iPlayerId );
	
	/**
	\brief Called when new server or client game session is started
	*/
	proto native void NewSession();
	
	/**
	\brief Debugging API for internal builds
	*/
	proto native void SetDebugHandling(EBackendRequest eRequest, EBackendDebugHandling eHandlingType);
}	

enum EBackendDebugHandling
{
	EBDH_NONE,
	EBDH_ERROR,
	EBDH_TIMEOUT
}

// -------------------------------------------------------------------------
