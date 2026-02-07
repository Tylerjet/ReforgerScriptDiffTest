
/** @file */


	//! Workshop code

	enum EApiCode
	{
		EACODE_ERROR_OK,
		EACODE_ERROR_UNKNOWN,
		EACODE_ERROR_ASSET_BLOCKED,
		EACODE_ERROR_ASSET_DELETED,
		EACODE_ERROR_ASSET_FIELD_IMMUTABLE,
		EACODE_ERROR_ASSET_NOT_CONTRIBUTED,
		EACODE_ERROR_ASSET_NOT_OWNED,
		EACODE_ERROR_ASSET_NOT_PUBLISHED,
		EACODE_ERROR_ASSET_PRIVATE,
		EACODE_ERROR_ASSET_VERSION_ALREADY_EXISTS,
		EACODE_ERROR_ASSET_TAG_NON_MATCHING_TYPE,
		EACODE_ERROR_ASSET_TOO_MANY_TAGS,
		EACODE_ERROR_COLLECTION_NOT_OWNED,
		EACODE_ERROR_COLLECTION_PRIVATE,
		EACODE_ERROR_COLLECTION_FULL,
		EACODE_ERROR_COMMENT_BLOCKED,
		EACODE_ERROR_COMMENT_NOT_OWNED,
		EACODE_ERROR_COMMENTS_ASSET_NOT_MATCHING,
		EACODE_ERROR_TAG_PROTECTED,
		EACODE_ERROR_UPLOAD_FIELD_LIMIT,
		EACODE_ERROR_UPLOAD_FILE_HASH_MISMATCH,
		EACODE_ERROR_UPLOAD_FILE_LIMIT,
		EACODE_ERROR_UPLOAD_FILE_SIZE_LIMIT,
		EACODE_ERROR_UPLOAD_FILE_SIZE_MISMATCH,
		EACODE_ERROR_UPLOAD_MANIFEST_INVALID,
		EACODE_ERROR_UPLOAD_MANIFEST_MISSING,
		EACODE_ERROR_UPLOAD_MANIFEST_SIZE_LIMIT,
		EACODE_ERROR_UPLOAD_NO_FILE_PROVIDED,
		EACODE_ERROR_UPLOAD_PART_LIMIT,
		//generic errors
		EACODE_ERROR_SERVICE_ERROR,
		EACODE_ERROR_VALIDATION_ERROR,
		EACODE_ERROR_UNAUTHORIZED,
		EACODE_ERROR_TOKEN_EXPIRED,
		EACODE_ERROR_RESOURCE_NOT_FOUND_ERROR,
		EACODE_ERROR_ALREADY_EXISTS,
		EACODE_ERROR_REQUEST_ERROR,
		EACODE_ERROR_REFERENCED_RESOURCE_NOT_FOUND_ERROR,
		EACODE_ERROR_NOT_IMPLEMENTED_ERROR,
		EACODE_ERROR_NOT_FOUND,
		EACODE_ERROR_METHOD_NOT_ALLOWED,
		EACODE_ERROR_INVALID_TOKEN_TYPE,
		EACODE_ERROR_INVALID_TOKEN,
		EACODE_ERROR_INTERNAL_SERVER_ERROR,
		EACODE_ERROR_CLIENT_ID_MISMATCH,
		EACODE_ERROR_ACCOUNT_LOCKED,
		//join errors
		EACODE_ERROR_MP_ROOM_NOT_FOUND,
		EACODE_ERROR_MP_ROOM_IS_FULL,
		EACODE_ERROR_PASSWORD_MISMATCH,
		EACODE_ERROR_P2P_USER_JOIN_BAN,
		EACODE_ERROR_DS_USER_JOIN_BAN,
		EACODE_ERROR_PLAYER_IS_BANNED,
		EACODE_ERROR_RENTED_SERVER_GAME_DATA_CORRUPTED,
		EACODE_ERROR_MAINTENANCE_IN_PROGRESS,
		EACODE_ERROR_USER_IS_BANNED_FROM_SHARED_GAME,
		EACODE_ERROR_NO_AVAILABLE_SERVER,
		EACODE_ERROR_DEDICATED_SERVER_COMMUNICATION_PROBLEM,
		EACODE_ERROR_RENTED_SERVER_GAME_LOCKED,
		EACODE_ERROR_SCENARIO_BLOCKED,
		EACODE_ERROR_TEMPORARY_HOSTING_ERROR,
		EACODE_ERROR_MP_ROOM_IS_NOT_JOINABLE,
		EACODE_ERROR_PLAYER_ALREADY_JOINED,
		//file errors
		EACODE_ERROR_FILE_LOAD_FAILED,
		EACODE_ERROR_FILE_SAVE_FAILED
	}

	//! Workshop item stats (values are bit flags!)
	enum EWorkshopItemState
	{
		EWSTATE_ONLINE,				// 2
		EWSTATE_OFFLINE,			// 4   	[Obsolete("Use Revision.IsDownloaded")]

		EWSTATE_UPLOADING,			// 8
		EWSTATE_DOWNLOADING,		// 16
		EWSTATE_OUTDATED,			// 32
		EWSTATE_CORRUPTED,			// 64
		EWSTATE_QUEUED,
		EWSTATE_FAVOURITE,
		EWSTATE_PURCHASED,
		EWSTATE_RECOMMENDED,
		EWSTATE_HIGHLIGHTED,
		EWSTATE_MYCREATION,			// 4096
		EWSTATE_BANNED	
	}

	//! Workshop report type
	enum EWorkshopReportType
	{
		EWREPORT_INAPPROPRIATE_CONTENT,
		EWREPORT_OFFENSIVE_LANGUAGE,
		EWREPORT_MISLEADING,
		EWREPORT_OTHER
	}

	enum EWorkshopOrderTypes
	{
		EWORDER_TYPE_NONE,
		EWORDER_TYPE_AVERAGE_RATING,
		EWORDER_TYPE_SUBSCRIBER_COUNT,
		EWORDER_TYPE_RATING_COUNT,
		EWORDER_TYPE_SIZE,
		EWORDER_TYPE_TYPENAME,
		EWORDER_TYPE_CREATED_AT,
		EWORDER_TYPE_UPDATED_AT,
		EWORDER_TYPE_ITEM_NAME,
		EWORDER_TYPE_FAVORITE,
		EWORDER_TYPE_RECENTLY_PLAYED
	}

	enum EWorkshopOrderDirections
	{
		EWORDER_DIR_ASC,
		EWORDER_DIR_DESC
	}
	
	enum EWorkshopFilterTypes
	{
		EWFILTER_TYPE_NONE,				//0
		EWFILTER_TYPE_SUBSCRIBED,		//1
		EWFILTER_TYPE_NOTSUBSCRIBED,	//2
		EWFILTER_TYPE_LOCAL,			//4
		EWFILTER_TYPE_ONLINE,			//8
		EWFILTER_TYPE_TYPE,				//16
		EWFILTER_TYPE_MULTIPLAYER,		//32
		EWFILTER_TYPE_SINGLEPLAYER		//64
	}

	enum EWorkshopItemResource
	{
		EWIRESOURCE_ONLINE,
		EWIRESOURCE_OFFLINE,
		EWIRESOURCE_SCENARIOS
	}

	enum EWorkshopItemAccessState
	{
		EWASTATE_OK,				//0
		EWASTATE_REPORTED,	//1
		EWASTATE_PRIVATE,		//2
		EWASTATE_BLOCKED,		//4
	}


// -------------------------------------------------------------------------
class PageParams extends JsonApiStruct
{	
	int limit;
	int offset;		
}

// -------------------------------------------------------------------------
class ImageScale
{
	proto native int Height();
	proto native int Width();
	proto native bool Download(BackendCallback callback);
	proto native string Path();
}
// -------------------------------------------------------------------------
class BackendImage
{
	//get the list of available scales
	proto native int GetScales(out notnull array<ImageScale> scales);
	
	//get a downloaded scale by its width
	//if no suitable scale is downloaded, it return the best downloaded candidate
	proto native ImageScale GetLocalScale(int width);
	
	//get a available scale by its width
	//if no suitable scale is available, it return the best availabe candidate
	proto native ImageScale GetScale(int width);
}

// -------------------------------------------------------------------------
// Workshop tag
class WorkshopTag
{

	void WorkshopTag()
	{
	}

	void ~WorkshopTag()
	{
	}
	

	/**
	\brief Working name (registered handle for search from script)
	*/
	proto native string Name();


	/**
	\brief Working id - todo:
	*/
	proto native string Id();
}

enum EPendingLoadState
{
	ELS_NONE,
	ELS_SCENARIOS,
	ELS_DEPENDENCIES
}

class Revision
{
	static const int INVALID_MAJOR_VERSION = -1;
	proto native string GetVersion();
	proto native string GetGameVersion();
	proto native void GetVersionArray(out notnull array<int> versionArray);
	proto native void GetGameVersionArray(out notnull array<int> versionArray);
	proto native int GetDependencies(out notnull array<Dependency> dependencies);
	proto native bool IsDownloaded();
	proto native int GetScenarios(out notnull array<MissionWorkshopItem> scenarios);
	proto bool GetPatchSize(out float size);
	proto native void ComputePatchSize(BackendCallback callback);
	proto native int CompareTo(Revision rev);
	/**
	\brief Describes which data are not loaded yet. 
	A bitmask which should be compared with EPendingLoadState.
	*/
	proto native int GetLoadFlags();

	static bool AreEqual(Revision a, Revision b)
	{
	 	return (a && b && a.CompareTo(b) == 0);
	}
	
	private void Revision(){};
	
	proto native int GetFiles(out notnull array<string> aFiles);
}

class WorkshopCatalogue
{
	proto native void RequestPage(BackendCallback callback, notnull PageParams params, bool clearCache);
	
	/**
	\brief Get current page number
	*/
	proto native int GetPage();

	/**
	\brief Get total item count on all pages
	*/
	proto native int GetTotalItemCount();

	/**
	\brief Get item count on actual page
	*/
	proto native int GetPageItemCount();

	/**
	\brief Get page count
	*/
	proto native int GetPageCount();
	
	//OBSOLETE
	/**
	\brief Set number of items per page
	*/
	proto native void SetPageItems( int count );
}

class DownloadableCatalogue extends WorkshopCatalogue
{	
	proto native void ScanOfflineItems();
	/**
	\brief Destroy items.
	*/
	proto native void Cleanup();
	
	proto native int GetBannedItems(out notnull array<string> items);
	
	proto native int GetDownloads(out notnull array<DownloadableItem> items);
}

class Dependency
{
	private void Dependency(){}
	void ~Dependency(){}
	
	proto native string GetID();
	
	proto native bool IsOffline();
	
	proto native string GetVersion();
	
	proto native string GetName();
	
	/**
	\brief Loads WorkshopItem from backend into cache
	*/
	proto native void LoadItem(BackendCallback callback);
	
	/**
	\brief Returns WorkshopItem if it is cashed somewhere in the workshop, if not the function returns null 
	*/
	proto native WorkshopItem GetCachedItem();
	
	proto native float TotalFileSize();
	
	proto native Revision GetRevision();
}


class WorkshopAuthor extends WorkshopCatalogue
{
	private void WorkshopAuthor()
	{
	}

	void ~WorkshopAuthor()
	{
	}
	
	proto native string Name();
	proto native bool IsBlocked();
	proto native void AddBlock(BackendCallback callback);
	proto native void RemoveBlock(BackendCallback callback);
	
	/*!
	Get page content.
	\param items Array of Workshop Items
	\return Current count of items on active page
	*/
	proto native int GetPageItems( out array<WorkshopItem> items );
}


class BaseWorkshopItem
{
	proto native string Name();
	proto native string Description();
}

class DownloadableItem extends BaseWorkshopItem
{
	proto native string Id();
	/**
	\brief Request download of this item
	*/
	proto native void Download(BackendCallback callback, Revision revision);
	
	/**
	\brief Request detail info
	*/
	proto native void AskDetail(BackendCallback callback);
	
	/**
	\brief Get Download/ Upload progress in range <0.0. .. 1.0>
	*/
	proto native float GetProgress();
	
	/**
	\brief Get thumbnail image
	*/
	proto native BackendImage Thumbnail();
	
	/**
	\brief Is the local version the latest one?
	*/
	proto native bool HasLatestVersion();
	
	
	proto native int GetRevisions(out notnull array<Revision> revisions);
	
	/**
	\brief Returns currently used revision or null if no revision is downloaded
	*/
	proto native Revision GetActiveRevision();
	
	/**
	\brief Returns currently downloading revision
	*/
	proto native Revision GetDownloadingRevision();
	
	/**
	\brief Cancel upload or download
	*/
	proto native void Cancel(BackendCallback callback);
	
	/**
	\brief Delete local copy of an asset
	*/
	proto native void DeleteLocally();
	
	proto native Revision GetPendingDownload();
	
	/**
	\brief Set script-defined persistent flags.
	*/
	proto native void SetScriptFlag(int iFlag);
	/**
	\brief Clear script-defined persistent flags.
	*/
	proto native void ClearScriptFlag(int iFlag); 
	/**
	\brief Get script-defined persistent flags.
	*/
	proto native int GetScriptFlag(); 
	
	proto native Revision GetLatestRevision();
	
	/**
	\brief Load revision's dependency list
	*/
	proto native void LoadDependencies(BackendCallback callback, notnull Revision revision);
	
	/**
	\brief Get local (non-workshop) revision
	*/
	proto native Revision GetLocalRevision();
	
	/**
	\brief Get summary
	*/
	proto native string Summary();
}



class MissionWorkshopItem extends BaseWorkshopItem
{
	proto native WorkshopItem GetOwner();
	
	proto native string GetOwnerId();
	
	proto native Class GetHeader();
	
	proto native BackendImage Thumbnail();
	
	proto native int GetPlayerCount();
	
	proto native bool IsFavorite();
	
	proto native void SetFavorite(bool isFavorite);
	
	proto native int GetTimeSinceLastPlay();
	
	proto native void Play();
	
	proto native void Host(DSConfig config);
	
	proto ResourceName Id();
}

// -------------------------------------------------------------------------
// Workshop item
class WorkshopItem extends DownloadableItem
{

	private void WorkshopItem()
	{
	}

	void ~WorkshopItem()
	{
	}

	/**
	\brief Get name of the author
	*/
	proto native WorkshopAuthor Author();
	/**
	\brief Subscribe this Item 
	*/
	proto native void Subscribe(BackendCallback callback);
	/**
	\brief Unsubscribe this Item 
	*/
	proto native void Unsubscribe(BackendCallback callback);
	/**
	\brief Is this item subscribed? 
	*/
	proto native bool IsSubscribed();

	/**
	\brief Load revision's scenarios
	*/
	proto native void LoadScenarios(BackendCallback callback, notnull Revision revision);
	/**
	\brief Call report (feedback) upon specific item
	*/
	proto native void Report( int /*EWorkshopReportType*/ eReport, string sMessage, BackendCallback callback );

	/**
	\brief Deletes an existing report from backend. When completed, the item will no longer report that it is reported.
	*/
	proto native void CancelReport( BackendCallback callback );

	/**
	\brief Loads existing report from backend. When done, use ... to get report data.
	*/
	proto native void LoadReport(BackendCallback callback);

	/**
	\brief Returns description of report.
	*/	
	proto native string GetReportDescription();
	
	/**
	\brief Returns type of report.
	*/
	proto native int GetReportType();


	/**
	\brief Get current state flags (EWorkshopItemState)
	*/
	proto native int GetStateFlags();

	/**
	\brief Get size of the package in bytes
	*/
	proto native float GetSizeBytes();

	/**
	\brief Get average rating - number in <0,5>
	*/
	proto native float AverageRating();
	
	/**
	\brief Reset my rating
	*/
	proto native void ResetRating( BackendCallback callback );
	
	/**
	\brief Set rating 
	*/
	proto native void Rate( bool upvote, BackendCallback callback );
	
	/**
	\brief Is my rating set
	*/
	proto native bool IsRatingSet();
	
	/**
	\brief Get rating value - true upvote, false downvote
	*/
	proto native bool MyRating();
	/**
	\brief Get rating count
	*/
	proto native int RatingCount();	
	/*!
	\brief Get 
	\param items Array of Workshop Items
	*/
	proto native int GetTags( out notnull array<WorkshopTag> items );

	/**
	\brief Returns true if process pending ATM (download, upload, delete)
	*/
	proto native bool IsProcessed();
	
	proto native void Enable(bool enable);
	proto native bool IsEnabled();
	
	proto native string License();
	proto native string LicenseText();
	
	/**
	\brief Get image gallery
	*/
	proto native int Gallery(out notnull array<BackendImage> gallery);
	
	proto native bool IsFavourite();
	
	proto native void SetFavourite(bool isFavourite);
	
	proto native void PauseDownload(BackendCallback callback);
	
	proto native void ResumeDownload(BackendCallback callback);
	
	/**
	\brief Is there enough space on the local storage
	*/
	proto native bool EnoughLocalSpace();
	/**
	\brief True if not only enabled, but actually loaded
	*/
	proto native bool IsLoaded();
		
	proto native bool HasAnyTag(notnull array<WorkshopTag> tags);
	
	/**
	\brief Returns access state flag -> EWorkshopItemAccessState
	*/
	proto native int GetAccessState();
	
	/**
	\brief Time in seconds since we downloaded this addon first time. Further updates do not affect this.
	*/
	proto native int GetTimeSinceFirstDownload();

	/**
	\brief Time since last time we played with this addon.
	*/
	proto native int GetTimeSinceLastPlay();
	
	/**
	\brief Resets TimeSinceLastPlay value.
	*/
	proto native void NotifyPlay();
	
	proto native static void SetThumbnailGridScale(int scale);
	
	proto native string GetBackendEnv();
	
	proto native string GetPath();
	
}


// -------------------------------------------------------------------------
// Callback interface for workshop - must exist for the duration of request!
class WorkshopCallback : Managed
{
	/**
	\brief Called when new page arrived
	\param page Index of page
	\param items Count of items on page (in case there are less than max per page)
	*/
	void OnPage( int page, int items )
	{
		//Print("[WorkshopCallback] OnPage, page=",page);
	}
	
	/**
	\brief Called when page items are iterated
	\param item Workshop item object
	\param index Position on current page
	*/
	void OnItem( WorkshopItem item, int index )
	{
		//Print("[WorkshopCallback] OnItem, index=",index);
	}

	/**
	\brief Request finished with success result
	\param code Code is type of EApiCode
	*/
	void OnSuccess( WorkshopItem item, int code )
	{
		//Print("[WorkshopCallback] OnSuccess: "+ g_Game.GetBackendApi().GetWorkshop().GetCode(code));
	}

	/**
	\brief Request finished with error result
	\param code Code is type of EApiCode
	*/
	void OnItemError( WorkshopItem item, int code )
	{
		//Print("[WorkshopCallback] OnError: "+ g_Game.GetBackendApi().GetWorkshop().GetCode(code));
	}
	/**
	\brief Request finished with error result
	\param code Code is type of EApiCode 	
	*/
	void OnError( int code )
	{
		
	}
	/**
	\brief Request not finished due to timeout
	*/
	void OnTimeoutItem( WorkshopItem item )
	{
		//Print("[WorkshopCallback] OnTimeout");
	}
	
	/**
	\brief Request not finished due to OnTimeout
	*/
	void OnTimeout()
	{
	}
	/**
	\brief Download has finished with success
	*/
	void OnDownloadSuccess( WorkshopItem item )
	{
	}
	

}


// -------------------------------------------------------------------------
// Workshop API access
class WorkshopApi extends DownloadableCatalogue
{

	private void WorkshopApi()
	{
	}

	private void ~WorkshopApi()
	{
	}


	//! Code to string
	string GetCode( int code )
	{
		return typename.EnumToString(EApiCode, code);
	}

	
	/**
	\brief Register new tag
	*/
	proto native WorkshopTag RegisterTag( string sName );
	/**
	\brief Get all known tags
	*/
	proto native int GetTags( out notnull array<WorkshopTag> tags );
	
	/**
	\brief Assign callback solver in script
	*/
	proto native void SetCallback( WorkshopCallback callback );

	/**
	\brief Get item by name
	*/
	proto native WorkshopItem GetByName( string name );

	/**
	\brief Get workshop quota (local storage) size in bytes
	*/
	proto native int GetQuotaLocal();

	/**
	\brief Get workshop free space (local storage) size in bytes
	*/
	proto native int GetFreeLocal();

	/**
	\brief Get workshop quota (cloud storage) size in bytes
	*/
	proto native int GetQuotaOnline();

	/**
	\brief Get workshop free space (cloud storage) size in bytes
	*/
	proto native int GetFreeOnline();
	
	proto native MissionWorkshopItem GetInGameScenario(ResourceName sResource);
	
	proto native WorkshopItem FindItem(string id);
	
	proto native int GetPageScenarios(out array<MissionWorkshopItem> items, int page, int pageSize);
	
	
	/**
	\brief Triggers OnSuccess when check for banned/up-to-date downloaded addons has finished
	*/
	proto native void OnItemsChecked(BackendCallback callback);
	
	proto native int GetOfflineItems(out notnull array<WorkshopItem> items);
	
	proto native bool NeedScan();
	/*!
	Get page content.
	\param items Array of Workshop Items
	\return Current count of items on active page
	*/
	proto native int GetPageItems( out array<WorkshopItem> items );
	
	proto native MissionWorkshopItem GetCurrentMission();
}

// -------------------------------------------------------------------------
