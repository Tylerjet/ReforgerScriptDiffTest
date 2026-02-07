
/** @file */
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
	EWSTATE_BANNED,
	EWSTATE_ABORTING_DOWNLOAD
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
	private void WorkshopTag()
	{
	}

	private void ~WorkshopTag()
	{
	}	

	/**
	\brief Working name (registered handle for search from script)
	*/
	proto native string Name();
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
	proto native string ChangeLog();
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
	
	proto native float GetTotalSize();
	
	proto native ERevisionAvailability GetAvailability();
	
	/**
	\brief Total size of all its files.
	*/
	proto native float GetSizeBytes();
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
	
	proto native void Download(BackendCallback callback);
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
	proto native int GetPageItems( out notnull array<WorkshopItem> items );
	proto native int GetOfflineItems(out notnull array<WorkshopItem> items);
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
	\brief Get download/update progress based on file operations in range <0.0. .. 1.0>
	*/
	proto native float GetProcessingProgress();
	
	/**
	\brief True if download is waiting for file processing.
	*/
	proto native bool IsProcessing();
	
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
	
	proto native void DeleteDownloadProgress();
	
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
	
	proto native string Author();
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
	\brief Get contributors of this Item
	\param contributors Array of contributors
	\return Count of contributors
	*/
	proto native int Contributors(out notnull array<WorkshopAuthor> contributors);
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
	\brief Load revision's changelog
	*/
	proto native void LoadChangelog(BackendCallback callback, notnull Revision revision);
	/**
	\brief Call report (feedback) upon specific item
	*/
	proto native void Report( EWorkshopReportType eReport, string sMessage, BackendCallback callback );

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
	[Obsolete("Use Revision.GetSizeBytes() instead.")]
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
		
	proto native bool IsFavorite();	
	proto native void SetFavorite(BackendCallback callback, bool isFavorite);
	
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
	
	/**
	\brief Set the prefed thumbnail size (width) that is automatically downloaded when called RequestPage 
	*/
	proto native static void SetThumbnailGridScale(int scale);
	
	proto native owned string GetBackendEnv();
	
	proto native bool IsReadyToRun();
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


	//! ApiCode to string
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
	\brief Get item by name
	*/
	proto native WorkshopItem GetByName( string name );
	
	/**
	\brief Find in-game MissionWorkshopItem by its MissionHeader config
	*/
	proto native MissionWorkshopItem GetInGameScenario(ResourceName sResource);
	
	/**
	\brief Find a WorkshopItem by ID in the local cache
	*/
	proto native WorkshopItem FindItem(string id);
	
	/**
	\brief Get scenarios of enabled WorkshopItems
	*/
	proto native int GetPageScenarios(out array<MissionWorkshopItem> items, int page, int pageSize);
	
	/**
	\brief Triggers OnSuccess when check for banned/up-to-date downloaded addons has finished
	*/
	proto native void OnItemsChecked(BackendCallback callback);
	
	/**
	\brief Get downloaded WorskhopItems
	*/
	proto native int GetOfflineItems(out notnull array<WorkshopItem> items);
	
	/**
	\brief True if local data were not loaded yet -> You can load them by ScanOfflineItems()
	*/
	proto native bool NeedScan();
	
	/*!
	Get page content.
	\param items Array of Workshop Items
	\return Current count of items on active page
	*/
	proto native int GetPageItems( out array<WorkshopItem> items );
	
	/**
	\brief Get downloaded WorskhopItems
	*/
	proto native MissionWorkshopItem GetCurrentMission();
}

// -------------------------------------------------------------------------
