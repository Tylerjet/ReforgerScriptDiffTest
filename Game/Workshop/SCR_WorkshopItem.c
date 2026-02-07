/*
!  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
!  ! If you use SCR_WorkshopItem, make sure you hold a strong reference (ref) to it:
!  ! Example: ref SCR_WorkshopItem m_Item;
!  ! The object is shared by nature and exists as long as at least something holds a strong
!  ! reference to it, if it is offline or as if any download is active.
!  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
*/

// Enum for possible issues of addons
enum EWorkshopItemProblem
{
	NO_PROBLEM = 0,
	BROKEN,
	DEPENDENCY_MISSING,
	DEPENDENCY_DISABLED,
	UPDATE_AVAILABLE,
	DEPENDENCY_OUTDATED
};

class SCR_WorkshopItem
{
	// friend class SCR_WorkshopItemAction
	
	// ---- Public variables ----
	ref ScriptInvoker m_OnChanged = new ref ScriptInvoker;				// (SCR_WorkshopItem item) - Called on any change and any event which affects state. Might be called on next frame!
	ref ScriptInvoker m_OnGetAsset = new ref ScriptInvoker;				// (SCR_WorkshopItem item) - Called on failure to load details too.
	ref ScriptInvoker m_OnDependenciesLoaded = new ref ScriptInvoker;	// (SCR_WorkshopItem item)
	ref ScriptInvoker m_OnScenariosLoaded = new ref ScriptInvoker;		// (SCR_WorkshopItem item)
	ref ScriptInvoker m_OnTimeout = new ref ScriptInvoker;				// (SCR_WorkshopItem item) - Called on any timeout event
	ref ScriptInvoker m_OnDownloadComplete = new ref ScriptInvoker;		// (SCR_WorkshopItem item) - Called when any download has been completed (including an update)
	ref ScriptInvoker m_OnOfflineStateChanged = new ref ScriptInvoker;	// (SCR_WorkshopItem item, bool newState) - Called when the addon was downloaded first time or deleted (OFFLINE flag changed its value)
	ref ScriptInvoker m_OnReportStateChanged = new ref ScriptInvoker;	// (SCR_WorkshopItem, bool newReported) - Called when reported state has changed
	ref ScriptInvoker m_OnMyReportLoaded = new ref ScriptInvoker;		// (SCR_WorkshopItem item) - called after report loading is done.
	
	// ---- Protected / Private ----
	
	protected ref WorkshopItem m_Item;	// Strong reference!
	protected ref Dependency m_Dependency;
	
	protected ref array<MissionWorkshopItem> m_aMissions;
	protected ref array<ref SCR_WorkshopItem> m_aDependencies;	// Strong references!
	protected ref array<SCR_WorkshopItem> m_aDependent = new array<SCR_WorkshopItem>;	// Weak refs - array of addons which depend on this addon
	protected ref array<Revision> m_aRevisions;
	
	// Name and id - just to check them in debugger
	#ifdef WORKSHOP_DEBUG
	protected string m_sName;
	protected string m_sId;
	#endif
	
	// Flags for state of partially loaded data
	protected bool m_bDetailsLoaded;	// Details are loaded - After LoadDetails
	
	// Backend callbacks
	protected ref SCR_WorkshopItemCallback_AskDetails m_CallbackAskDetails;
	protected ref SCR_WorkshopCallbackBase m_CallbackLoadGallery;
	protected ref SCR_WorkshopItemCallback_LoadDependencies m_CallbackLoadDependencies;
	protected ref SCR_WorkshopItemCallback_LoadScenarios m_CallbackLoadScenarios;
	protected ref SCR_WorkshopItemCallback_LoadMyReport m_CallbackLoadMyReport;
	
	// Various state flags
	protected bool m_bMyRating;
	protected bool m_bMyRatingSet;
	protected bool m_bFavourite;
	protected bool m_bSubscribed;
	protected bool m_bChanged;		// Checked every frame, if something changes then this flag can be set to OnChanged script invoker is called during update.
	protected bool m_bRequestFailed;	// Set to true on HTTP request failures
	protected bool m_bOffline;
	protected int m_iAccessState;
	
	// Data loading flags
	protected bool m_bWaitingLoadDetails;		// With current API it's always true after a LoadDetails call, since we can't know when it's done for sure
	protected float m_fPrevDownloadProgress;	// Used to detect when download progress changes
	
	// Actions
	protected ref SCR_WorkshopItemActionDownload m_ActionDownload;
	protected ref SCR_WorkshopItemActionReport m_ActionReport;
	protected ref SCR_WorkshopItemActionCancelReport m_ActionCancelReport;
	protected ref SCR_WorkshopItemActionComposite m_ActionDependency; // Composite action we have started for dependencies
	
	
	//-----------------------------------------------------------------------------------------------
	// 						P U B L I C   M E T H O D S 
	//-----------------------------------------------------------------------------------------------
	
	//! Use these only if absoulutely necessary to intarract with game API.
	WorkshopItem GetWorkshopItem() { return m_Item; }
	Dependency GetDependency() { return m_Dependency; }
	
	//! Getters for loading state
	bool GetItemDataLoaded() { return m_Item != null; }
	bool GetDetailsLoaded() { return m_bDetailsLoaded; }				// GetAsset request was completed, doesn't mean that anything else related to it is loaded
	bool GetScenariosLoaded() { return m_aMissions != null; }			// Scenarios were loaded after GetAsset request
	bool GetRevisionsLoaded() { return m_aRevisions != null; }			// Revisions were loaded after GetAsset request
	bool GetDependenciesLoaded() { return m_aDependencies != null; }	// Latest dependencies were loaded after GetAsset request
	bool GetRequestFailed() { return m_bRequestFailed; }				// True when any request has failed
	
	//-----------------------------------------------------------------------------------------------
	//! Logs all properties of the object into console
	void LogState()
	{
		bool myRating, myRatingSet;
		GetMyRating(myRatingSet, myRating);
		
		string description = GetDescription();
		if (description.Length() > 32)
		{
			description = description.Substring(0, 32);
			description = description + " <truncated>";
		}
		
		string summary = GetSummary();
		if (summary.Length() > 32)
		{
			summary = summary.Substring(0, 32);
			summary = summary + " <truncated>";
		}
		
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		_print("\nLogState:");
		_print(string.Format("   Name:                %1", GetName()));
		_print(string.Format("   ID:                  %1", GetId()));
		_print(string.Format("   Dependency:          %1", m_Dependency));
		_print(string.Format("   WorkshopItem:        %1", m_Item));
		_print(string.Format("   Subscribed:          %1", GetSubscribed()));
		_print(string.Format("   Enabled:             %1", GetEnabled()));
		_print(string.Format("   Online:              %1", GetOnline()));
		_print(string.Format("   Offline:             %1", GetOffline()));
		_print(string.Format("   Blocked:             %1", GetBlocked()));
		_print(string.Format("   ReportedByMe:        %1", GetReportedByMe()));
		_print(string.Format("   CurrentLocalVersion: %1", GetCurrentLocalVersion()));
		_print(string.Format("   LatestVersion:       %1", GetLatestVersion()));
		_print(string.Format("   ItemDataLoaded:      %1", GetItemDataLoaded()));
		_print(string.Format("   DetailsLoaded:       %1", GetDetailsLoaded()));
		_print(string.Format("   ScenariosLoaded:     %1", GetScenariosLoaded()));
		_print(string.Format("   DependenciesLoaded:  %1", GetDependenciesLoaded()));
		_print(string.Format("   RequestFailed:       %1", GetRequestFailed()));
		//_print(string.Format("   OfflineAndUpToDate:  %1", GetIsOfflineAndUpToDate()));
		_print(string.Format("   AuthorName:          %1", GetAuthorName()));
		_print(string.Format("   MyRating:            %1", myRating));
		_print(string.Format("   MyRatingSet:         %1", myRatingSet));
		_print(string.Format("   Favourite:           %1", GetFavourite()));
		_print(string.Format("   Corrupted:           %1", GetCorrupted()));
		_print(string.Format("   Description:         %1", description));
		_print(string.Format("   Summary:             %1", summary));
		_print(string.Format("   RatingCount:         %1", GetRatingCount()));
		_print(string.Format("   SizeBytes:           %1", GetSizeBytes()));
		_print(string.Format("   UpdateAvailable:     %1", GetUpdateAvailable()));
		_print(string.Format("   EnabledExternally:   %1", mgr.GetAddonEnabledExternally(this)));
		_print(string.Format("   TimeSinceLastPlay:   %1, %2", GetTimeSinceLastPlay(), SCR_Global.GetTimeSinceEventImprecise(GetTimeSinceLastPlay())));
		_print(string.Format("   TimeSinceFirstDl:    %1, %2", GetTimeSinceFirstDownload(), SCR_Global.GetTimeSinceEventImprecise(GetTimeSinceFirstDownload())));
		
		//if (m_Item)
		//_print(string.Format("   HasBackendThumbnail: %1", m_Item.HasBackendThumbnail()));
			
		_print(string.Format("   AnyDepDisabled:      %1", GetEnabledAndAnyDependencyDisabled()));
		_print(string.Format("   AnyDepOutdated:      %1", GetAnyDependencyUpdateAvailable()));
		_print(string.Format("   AnyDepMissing:       %1", GetAnyDependencyMissing()));
		
		_print(string.Format("   Dependencies:        %1", dependencies.Count()));
		foreach (SCR_WorkshopItem dep : dependencies)
		{
		_print(string.Format("      - %1, Offline: %2, Update Available: %3", dep.GetName(), dep.GetOffline(), dep.GetUpdateAvailable()));
		}
		
		// todo scenarios, dependencies
		
		_print("\n\n");
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Loads details from backend, or if they are already loaded, calls the callbacks immediately
	void LoadDetails()
	{
		#ifdef WORKSHOP_DEBUG
		_print("LoadDetails");
		#endif
		
		if (m_Item || m_Dependency)
		{
			Internal_LoadDetails();
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of scenarios
	void GetScenarios(array<MissionWorkshopItem> scenarios)
	{	
		if (!m_aMissions)
			return;
		
		scenarios.Clear();
		scenarios.Copy(m_aMissions);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of dependencies
	array<ref SCR_WorkshopItem> GetLatestDependencies()
	{
		array<ref SCR_WorkshopItem> dependencies = new array<ref SCR_WorkshopItem>;
		
		if (!m_aDependencies)
			return dependencies;
		
		foreach (auto a : m_aDependencies)
			dependencies.Insert(a);
		
		return dependencies;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of dependendent addons
	array<ref SCR_WorkshopItem> GetDependentAddons()
	{
		array<ref SCR_WorkshopItem> dependent = new array<ref SCR_WorkshopItem>;
		
		if (!m_aDependent)
			return dependent;
		
		foreach (auto a : m_aDependent)
		{
			if (a)
				dependent.Insert(a);
		}
		
		return dependent;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetLatestVersion()
	{
		Revision rev = GetLatestRevision();
		
		if (!rev)
			return string.Empty;
		
		return rev.GetVersion();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when offline and latest version matches the current version
	/*
	bool GetIsOfflineAndUpToDate()
	{
		return GetOffline() && GetLatestVersion() == GetCurrentLocalVersion(); // todo it might not work until actual versions are downloaded
	}
	*/
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of versions
	array<string> GetVersions()
	{	
		array<string> versions = new array<string>;
		
		if (!GetRevisionsLoaded())
			return versions;
		
		foreach (auto rev : m_aRevisions)
		{
			versions.Insert(rev.GetVersion());
		}
		
		return versions;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetMyRating(bool newRating)
	{
		if (!m_Item)
			return;

		m_bMyRatingSet = true;
		m_bMyRating = newRating;
		
		#ifdef WORKSHOP_DEBUG
		_print("WorkshopItem.Rate(%1)", newRating);
		#endif
				
		m_Item.Rate(newRating, null);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetMyRating()");
		#endif
			
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void ResetMyRating()
	{
		if (!m_Item)
			return;
		
		m_bMyRatingSet = false;
		
		#ifdef WORKSHOP_DEBUG
		_print("WorkshopItem.ResetRating()");
		#endif
		
		m_Item.ResetRating(null);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: ResetMyRating()");
		#endif
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void GetMyRating(out bool ratingSet, out bool rating)
	{
		ratingSet = m_bMyRatingSet;
		rating = m_bMyRating;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void SetFavourite(bool favourite)
	{
		if (!m_Item)
			return;
		
		m_bFavourite = true;
		
		m_Item.SetFavourite(favourite);
		m_bFavourite = favourite;
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetFavourite()");
		#endif
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool GetFavourite()
	{
		return m_bFavourite;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetSubscribed(bool subscribe)
	{
		if (!m_Item)
			return;
		
		if (subscribe)
			m_Item.Subscribe(null);
		else
			m_Item.Unsubscribe(null);
		
		m_bSubscribed = subscribe;
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Subscribe()");
		#endif
		
		SetChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetSubscribed()
	{
		return m_bSubscribed;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetEnabled(bool enable)
	{
		if (!GetOffline())
			return;
		
		m_Item.Enable(enable);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetEnabled()");
		#endif
		
		if (enable)
			m_Item.NotifyPlay();
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Enables/disables all downloaded dependencies
	void SetDependenciesEnabled(bool enable)
	{
		if (!m_aDependencies)
			return;
		
		foreach (auto dep : m_aDependencies)
		{
			if (dep.GetOffline() && dep.GetEnabled() != enable)
			{
				dep.SetEnabled(enable);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetEnabled()
	{
		if (!m_Item)
			return false;
		
		if (!GetOffline())
			return false;
		
		return m_Item.IsEnabled();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! True when addon is loaded by engine. It means that the game is already running with this mod.
	bool GetLoaded()
	{
		if (!m_Item)
			return false;
		
		return m_Item.IsLoaded();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetBlocked()
	{
		if (m_Item)
		{
			int accessState = m_Item.GetAccessState();
			return accessState & EWorkshopItemAccessState.EWASTATE_BLOCKED;
		}
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetReportedByMe()
	{
		if (m_Item)
			return m_Item.GetAccessState() & EWorkshopItemAccessState.EWASTATE_REPORTED;
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when item is restricted for any reason (blocked or reported)
	bool GetRestricted()
	{
		return GetBlocked() || GetReportedByMe();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool DeleteLocally()
	{
		if (!m_Item)
			return false;
		
		if (!GetOffline())
			return true; // Already deleted
		
		m_Item.DeleteLocally();
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: DeleteLocally()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetName()
	{
		if (m_Item)
			return m_Item.Name();
		else if (m_Dependency)
			return m_Dependency.GetName();
		else
			return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetId()
	{
		if (m_Item)
			return m_Item.Id();
		else if (m_Dependency)
			return m_Dependency.GetID();
		else
			return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetAuthorName()
	{
		if (m_Item)
			return m_Item.AuthorName();
		else
			return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! True when the item is stored in the backend
	bool GetOnline()
	{
		if (m_Item)
			return m_Item.GetStateFlags() & EWorkshopItemState.EWSTATE_ONLINE;
		else if (m_Dependency)
			return true; // TODO Apparently, if it's a dependency, then it's online
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! True when we have the item on our local storage
	bool GetOffline()
	{
		if (m_Item)
			return m_Item.GetStateFlags() & EWorkshopItemState.EWSTATE_OFFLINE;
		else if (m_Dependency)
			return m_Dependency.IsOffline();
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the revision which we currently have on the local storage
	string GetCurrentLocalVersion()
	{
		if (m_Item)
		{
			if (!GetOffline())
			{
				return string.Empty;
			}
			else
			{
				Revision rev = m_Item.GetActiveRevision();
				
				if (!rev)
					return string.Empty;
				
				return rev.GetVersion();
			}
		}
		else
			return string.Empty;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true if current local version matches version of dependency
	bool GetCurrentLocalVersionMatchDependency()
	{
		if (!m_Item || !m_Dependency)
			return false;
			
		// Does active version match required?
		string ActiveV = GetCurrentLocalVersion();
		string RequiredV = m_Dependency.GetVersion();
		
		return (ActiveV == RequiredV);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool GetCorrupted()
	{
		if (m_Item)
			return (m_Item.GetStateFlags() & EWorkshopItemState.EWSTATE_CORRUPTED);
		else if (m_Dependency)
			return false; // TODO this is not valid for dependency object
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	BackendImage GetThumbnail()
	{		
		if (!m_Item)
			return null;
		
		return m_Item.Thumbnail();
	}
	
	//-----------------------------------------------------------------------------------------------
	array<BackendImage> GetGallery()
	{
		array<BackendImage> gallery = {};
		
		if (!m_Item)
			return gallery;
		
		m_Item.Gallery(gallery);		
		return gallery;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetDescription()
	{
		if (!m_Item)
			return string.Empty;
		
		return m_Item.Description();
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetSummary()
	{
		if (!m_Item)
			return string.Empty;
		
		return m_Item.Summary();
	}
	
	//-----------------------------------------------------------------------------------------------
	int GetRatingCount()
	{
		if (!m_Item)
			return 0;
		
		return m_Item.RatingCount();
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetSizeBytes()
	{
		if (m_Item)
			return m_Item.GetSizeBytes();
		else if(m_Dependency)
			return m_Dependency.TotalFileSize();
		else
			return 0;
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetAverageRating()
	{
		if (!m_Item)
			return 0;
		
		return m_Item.AverageRating();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns state of the download process
	void GetDownloadState(out bool inProgress, out bool paused, out float progress, out string targetVersion)
	{
		if (!m_ActionDownload || !m_Item)
		{
			inProgress = false;
			paused = false;
			progress = 0;
			targetVersion = string.Empty;
			return;
		}
		
		inProgress = m_ActionDownload.IsActive() || m_ActionDownload.IsPaused();
		paused = m_ActionDownload.IsPaused();
		progress = m_Item.GetProgress();
		targetVersion = m_ActionDownload.GetTargetVersion();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns current download action
	SCR_WorkshopItemActionDownload GetDownloadAction()
	{
		return m_ActionDownload;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns the current composite action performed for dependencies of this addon
	SCR_WorkshopItemActionComposite GetDependencyCompositeAction()
	{
		return m_ActionDependency;
	}
		
	//-----------------------------------------------------------------------------------------------
	//! Returns true when we have an old version offline, and the new version
	bool GetUpdateAvailable()
	{
		string currentLocalVersion = GetCurrentLocalVersion();
		array<string> versions = GetVersions(); // Latest version is always last in the array
		
		if (currentLocalVersion.IsEmpty() || versions.IsEmpty())
			return false;
		
		int id = versions.Find(currentLocalVersion);
		
		if (id == -1)	// Our local version was not found in the array - why??
			return false;
		
		int versionCount = versions.Count();
		
		if (id == versionCount - 1) // We are at latest version
			return false;
		else
			return true; // Our version is not latest
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns dependencies which are missing.
	array<ref SCR_WorkshopItem> GetMissingDependencies()
	{	
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		return SCR_AddonManager.SelectItemsBasic(dependencies, EWorkshopItemQuery.NOT_OFFLINE);
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetAnyDependencyMissing()
	{
		if (!m_aDependencies)
			return false;
		
		bool count = SCR_AddonManager.CountItemsBasic(m_aDependencies, EWorkshopItemQuery.NOT_OFFLINE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetAnyDependencyUpdateAvailable()
	{
		if (!m_aDependencies)
			return false;
		
		bool count = SCR_AddonManager.CountItemsBasic(m_aDependencies, EWorkshopItemQuery.UPDATE_AVAILABLE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true if any dependency is offline and disabled
	bool GetEnabledAndAnyDependencyDisabled()
	{
		if (!m_aDependencies)
			return false;
		
		if (!GetEnabled())
			return false;
		
		bool count = SCR_AddonManager.CountItemsAnd(m_aDependencies, EWorkshopItemQuery.NOT_ENABLED | EWorkshopItemQuery.OFFLINE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns highest priority issue of this addon if it's downloaded,
	//! Addon might have all issue types at once, but this method returns only the most important issue
	//! In the order most suitable for UI
	EWorkshopItemProblem GetHighestPriorityProblem()
	{
		if (!GetOffline())
			return EWorkshopItemProblem.NO_PROBLEM;
		
		if (GetAnyDependencyMissing())
			return EWorkshopItemProblem.DEPENDENCY_MISSING;
		else if (GetEnabledAndAnyDependencyDisabled())
			return EWorkshopItemProblem.DEPENDENCY_DISABLED;
		else if (GetUpdateAvailable())
			return EWorkshopItemProblem.UPDATE_AVAILABLE;
		else if (GetAnyDependencyUpdateAvailable())
			return EWorkshopItemProblem.DEPENDENCY_OUTDATED;
		
		return EWorkshopItemProblem.NO_PROBLEM;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns dependencies which can be updated.
	array<ref SCR_WorkshopItem> GetOutdatedDependencies()
	{	
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		if (dependencies.IsEmpty())
			return new array<ref SCR_WorkshopItem>;
		
		return SCR_AddonManager.SelectItemsBasic(dependencies, EWorkshopItemQuery.UPDATE_AVAILABLE);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	int GetTimeSinceLastPlay()
	{
		if (!m_Item)
			return -1;
		
		return m_Item.GetTimeSinceLastPlay();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	int GetTimeSinceFirstDownload()
	{
		if (!m_Item)
			return -1;
		
		return m_Item.GetTimeSinceFirstDownload();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Starts download of a specific version.
	//! Returns an action object, use it to control the download process or
	//! To get notified about its events.
	//!
	//! !!! If a download of same version is already running, it doesn't create a new action
	//! !!! But returns a previous one instead.
	//! 
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	//! ! You must call Activate() of the returned action yourself.
	//! ! This is done this way so that you can subscribe to events before the action is started.
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	SCR_WorkshopItemActionDownload Download(string targetVersion)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Download: %1", targetVersion));
		#endif
		
		if (!m_Item && !m_Dependency)
			return null;
		
		if (m_ActionDownload)
		{
			if (m_ActionDownload.GetTargetVersion() == targetVersion)
			{
				// Same kind of download is already running
				return m_ActionDownload; // TODO if action was canceled or failed, create a new one
			}
			else
			{
				// Another download is running, but downloading a different version
				m_ActionDownload.Cancel();
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				
				m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: false, targetVersion);
				SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
				return m_ActionDownload;
			}
		}
		else
		{
			m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: false, targetVersion);
			SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
			return m_ActionDownload;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload DownloadLatestVersion()
	{
		#ifdef WORKSHOP_DEBUG
		_print("DownloadLatestVersion()");
		#endif
		
		if (!m_Item && !m_Dependency)
			return null;
		
		if (m_ActionDownload)
		{
			string latestVersion = this.GetLatestVersion(); // Might return empty string if revs not loaded!
			if (m_ActionDownload.GetTargetedAtLatestVersion() || m_ActionDownload.GetTargetVersion() == latestVersion)
			{
				// We are either already downloading the latest version,
				// or a download of latest version was scheduled internally in the action
				return m_ActionDownload; // TODO if action was canceled or failed, create a new one
			}
			else
			{
				// Another download is running, but downloading a different version
				m_ActionDownload.Cancel();
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				
				m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: true);
				SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
				return m_ActionDownload;
			}
		}
		else
		{
			m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: true);
			SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
			return m_ActionDownload;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Starts downloading latest version of dependencies which are offline
	//! !!! If a previous same action is running, it doesn't create a new action but
	//! !!! Returns a previous action instead.
	SCR_WorkshopItemActionDownloadDependenciesLatest DownloadDependenciesLatest(array<ref SCR_WorkshopItem> dependencies)
	{
		#ifdef WORKSHOP_DEBUG
		_print("DownloadDependenciesLatest()");
		#endif
		
		if (!m_Item)
			return null;
		
		if (m_ActionDependency)
		{
			auto action = SCR_WorkshopItemActionDownloadDependenciesLatest.Cast(m_ActionDependency);
			if (action)
			{
				// Download action with dependencies is running
				return action;
			}
			else
			{
				// Another kind of action with dependencies is running, cancel it
				m_ActionDependency.Cancel();
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				
				action = new SCR_WorkshopItemActionDownloadDependenciesLatest(this, dependencies);
				m_ActionDependency = action;
				return action;
			}
		}
		else
		{
			auto action = new SCR_WorkshopItemActionDownloadDependenciesLatest(this, dependencies);
			m_ActionDependency = action;
			return action;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Pauses current download, if possible
	bool PauseDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Pause();
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Cancels current download, if possible
	bool CancelDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Cancel();
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Resumes current download, if it's already started
	bool ResumeDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Resume();
		}
		
		return false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Sends a report about this workshop item
	//!
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	//! ! You must call Activate() of the returned action yourself.
	//! ! This is done this way so that you can subscribe to events before the action is started.
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	SCR_WorkshopItemActionReport Report(EWorkshopReportType eReport, string sMessage)
	{
		if (!m_Item)
			return null;
		
		if (m_ActionReport)
		{
			// We can't abort it or change while it's being sent anyway,
			// Return the existing action
			return m_ActionReport;
		}
		else
		{
			m_ActionReport = new SCR_WorkshopItemActionReport(this, eReport, sMessage);
			return m_ActionReport;
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Loads report of current user from backend. Subscribe to m_OnMyReportLoaded to get result.
	void LoadReport()
	{
		if (!m_Item)
			return;
		
		if (!m_CallbackLoadMyReport)
		{
			m_CallbackLoadMyReport = new SCR_WorkshopItemCallback_LoadMyReport();
			m_CallbackLoadMyReport.m_OnSuccess.Insert(Callback_LoadMyReport_OnSuccess);
			m_CallbackLoadMyReport.m_OnTimeout.Insert(Callback_LoadMyReport_OnTimeout);
			m_CallbackLoadMyReport.m_OnError.Insert(Callback_LoadMyReport_OnError);
		}
		
		m_Item.LoadReport(m_CallbackLoadMyReport);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns our own report. We must load it first with LoadMyReport.
	//! How the function behaves immediately relative to data passed through Report() is undefined.
	void GetReport(out EWorkshopReportType reportType, out string reportDescription)
	{
		if (!m_Item)
		{
			reportType = EWorkshopReportType.EWREPORT_OTHER;
			reportDescription = string.Empty;
			return;
		}
		
		reportType = m_Item.GetReportType();
		reportDescription = m_Item.GetReportDescription();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Sends a cancel report about this workshop item
	SCR_WorkshopItemActionCancelReport CancelReport()
	{
		if (!m_Item)
			return null;
		
		if (m_ActionReport)
		{
			// We can't abort it or change while it's being sent anyway,
			// Return the existing action
			return m_ActionCancelReport;
		}
		else
		{
			m_ActionCancelReport = new SCR_WorkshopItemActionCancelReport(this);
			return m_ActionCancelReport;
		}
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	// 					P R O T E C T E D   /   P R I V A T E
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	//! Finds revision object by string version
	protected Revision FindRevision(string version)
	{
		if (!m_aRevisions)
			return null;
		
		foreach (Revision r : m_aRevisions)
		{
			if (r.GetVersion() == version)
				return r;
		}
		
		return null;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected Revision GetLatestRevision()
	{
		if (!m_aRevisions)
			return null;
		
		if (m_aRevisions.Count() == 0)
			return null;
		
		return m_aRevisions[m_aRevisions.Count() - 1];
	}
	
	
	//-----------------------------------------------------------------------------------------------
	// 					C A L L B A C K S
	//-----------------------------------------------------------------------------------------------
	
	
	//  --- Ask Details ---
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_AskDetails_OnGetAsset()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_AskDetails_OnGetAsset()");
		#endif
		
		TryLoadItemFromDependency();
		
		TryLoadRevisions();
		
		if (!m_aRevisions || m_aRevisions.IsEmpty())
		{
			#ifdef WORKSHOP_DEBUG
			_print("Callback_AskDetails_OnGetAsset(): Received no revisions", LogLevel.ERROR);
			#endif
		}
		else
		{
			#ifdef WORKSHOP_DEBUG
			_print("Callback_AskDetails_OnGetAsset(): Received revisions:");
			array<string> versions = GetVersions();
			foreach (string ver : versions)
			{
				_print(string.Format("Callback_AskDetails_OnGetAsset():   %1", ver));
			}
			#endif
			
			if (!m_CallbackLoadDependencies)
			{
				m_CallbackLoadDependencies = new SCR_WorkshopItemCallback_LoadDependencies;
				m_CallbackLoadDependencies.m_OnSuccess.Insert(Callback_LoadDependencies_OnSuccess);
				m_CallbackLoadDependencies.m_OnTimeout.Insert(Callback_LoadDependencies_OnTimeout);
			}
			
			if (!m_CallbackLoadScenarios)
			{
				m_CallbackLoadScenarios = new SCR_WorkshopItemCallback_LoadScenarios;
				m_CallbackLoadScenarios.m_OnSuccess.Insert(Callback_LoadScenarios_OnSuccess);
				m_CallbackLoadScenarios.m_OnTimeout.Insert(Callback_LoadScenarios_OnTimeout);
			}
			
			if (m_Item)
			{
				Revision latestRevision = GetLatestRevision();
				m_Item.LoadDependencies(m_CallbackLoadDependencies, latestRevision);
				m_Item.LoadScenarios(m_CallbackLoadScenarios, latestRevision);
			}
		}
		
		m_bDetailsLoaded = true;
		m_OnGetAsset.Invoke(this);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Callback_AskDetails_OnGetAsset");
		#endif
		
		SetChanged();
		
		m_bWaitingLoadDetails = false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_AskDetails_OnTimeout()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_AskDetails_OnTimeout()");
		#endif
		
		m_bWaitingLoadDetails = false;
		
		m_OnTimeout.Invoke(this);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_AskDetails_OnError(SCR_WorkshopCallbackBase callback, EBackendError code, int restCode, int apiCode )
	{
		m_bRequestFailed = true;
		m_bWaitingLoadDetails = false;
		
		// We still have received a response, but with an error.
		m_OnGetAsset.Invoke(this);
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadDependencies_OnSuccess()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadDependencies_OnSuccess()");
		#endif
		
		TryLoadDependencies(); // For now we support only dependencies of latest version
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadDependencies_OnTimeout()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadDependencies_OnTimeout()");
		#endif
		
		m_OnTimeout.Invoke(this);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadScenarios_OnSuccess()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadScenarios_OnSuccess()");
		#endif
		
		TryLoadScenarios();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadScenarios_OnTimeout()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadScenarios_OnTimeout()");
		#endif
		
		m_OnTimeout.Invoke(this);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadMyReport_OnSuccess()
	{
		m_OnMyReportLoaded.Invoke(this);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadMyReport_OnTimeout()
	{
		m_OnTimeout.Invoke(this);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadMyReport_OnError()
	{
		// Error might happen if there is no report at all
		// Let's report success anyway
		m_OnMyReportLoaded.Invoke(this);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadItemFromDependency()
	{
		// Try to get WorkshopItem from Dependency, if we have only Dependency
		if (!m_Item && m_Dependency)
		{
			m_Item = m_Dependency.GetCachedItem();
			
			if (m_Item)
			{
				UpdateStateFromWorkshopItem();
			}
			
			#ifdef WORKSHOP_DEBUG
			if (m_Item)
				_print(string.Format("WorkshopItem was missing, but was loaded: %1", m_Item.Name()));
			#endif
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadRevisions(bool log = true)
	{
		if (!m_Item)
			return;
		
		array<Revision> revisionsTemp = new array<Revision>;
		m_Item.GetRevisions(revisionsTemp);
		
		if (revisionsTemp.IsEmpty())
		{
			#ifdef WORKSHOP_DEBUG
			if (log)
				_print(string.Format("Revisions are not loaded yet. Revision array is empty."));
			#endif
			
			return;
		}
			
		// The revision array is not empty, so we can store it
		// When m_aRevisions is not null, it means revisions are loaded
		m_aRevisions = revisionsTemp;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadDependencies(bool log = true)
	{
		#ifdef WORKSHOP_DEBUG
		if (log)
			_print("TryLoadDependencies()");
		#endif
		
		if (!m_aRevisions)
		{
			#ifdef WORKSHOP_DEBUG
			if (log)
				_print("TryLoadDependencies(): m_aRevisions is null");
			#endif
			
			return;
		}
			
		// Get array of dependencies
		// Register every dependency in the addon manager
		auto latestRevision = m_aRevisions[m_aRevisions.Count() - 1];
		array<Dependency> dependencies = new array<Dependency>;
		latestRevision.GetDependencies(dependencies);
		
		if (!m_aDependencies)
			m_aDependencies = new array<ref SCR_WorkshopItem>;
		
		m_aDependencies.Clear();
		
		array<SCR_WorkshopItem> registeredDependencies = new array<SCR_WorkshopItem>;
		
		foreach (Dependency dep : dependencies)
		{
			SCR_WorkshopItem registeredItem = SCR_AddonManager.GetInstance().Register(dep);
			
			if (registeredItem)
				registeredDependencies.Insert(registeredItem);
		}
		
		foreach (SCR_WorkshopItem dep : registeredDependencies)
		{
			this.RegisterDependency(dep);
			dep.RegisterDependent(this);
		}
		
		m_OnDependenciesLoaded.Invoke(this);
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadScenarios(bool log = true)
	{
		#ifdef WORKSHOP_DEBUG
		if (log)
			_print("TryLoadScenarios()");
		#endif
		
		if (!m_aRevisions)
		{
			#ifdef WORKSHOP_DEBUG
			if (log)
				_print("TryLoadScenarios(): m_aRevisions is null");
			#endif
			return;
		}
			
		
		// Load scenarios of latest revision
		Revision latestRevision = m_aRevisions[m_aRevisions.Count() - 1];
		
		m_aMissions = new array<MissionWorkshopItem>;
		latestRevision.GetScenarios(m_aMissions);
		
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("TryLoadScenarios(): Received %1 scenarios", m_aMissions.Count()));
		_print("OnChanged: Callback_AskDetails_OnGetScenarios");
		#endif
		
		SetChanged();
		
		m_OnScenariosLoaded.Invoke(this);
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void RegisterDependency(SCR_WorkshopItem item)
	{
		int id = m_aDependencies.Find(item);
		if (id == -1)
			m_aDependencies.Insert(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterDependency(SCR_WorkshopItem item)
	{
		m_aDependencies.RemoveItem(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void RegisterDependent(SCR_WorkshopItem item)
	{
		int id = m_aDependent.Find(item);
		if (id == -1)
			m_aDependent.Insert(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterDependent(SCR_WorkshopItem item)
	{
		m_aDependent.RemoveItem(item);
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	
	
	
	
	
	
	//-----------------------------------------------------------------------------------------------
	// 			I N T E R A C T I O N S   W I T H   W O R K S H O P   A P I 
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_StartDownload(string targetVersion, BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_StartDownload(): %1, %2", targetVersion, callback));
		#endif
		
		// Check item
		if (!m_Item)
		{
			#ifdef WORKSHOP_DEBUG
			_print("Internal_StartDownload(): m_Item is null!");
			#endif
			return false;
		}
		
		// Check revisions
		Revision rev = this.FindRevision(targetVersion);
		if (!rev)
		{
			#ifdef WORKSHOP_DEBUG
			_print("Internal_StartDownload(): Revision was not found!");
			#endif
			return false;
		}
			
		
		// Check privileges
		if (!SCR_AddonManager.GetInstance().GetUgcPrivilege())
		{
			#ifdef WORKSHOP_DEBUG
			_print("Internal_StartDownload(): UGC privileges were not checked or false!");
			#endif
			return false;
		}
		
		
		#ifdef WORKSHOP_DEBUG
		_print("Internal_StartDownload(): Calling WorkshopItem.Download ...");
		#endif
		
		m_Item.Download(callback, rev);
		
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_PauseDownload()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Internal_PauseDownload()");
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.PauseDownload();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_CancelDownload()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Internal_CancelDownload()");
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.Cancel();
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_ResumeDownload(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print("Internal_ResumeDownload()");
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.ResumeDownload(callback);
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_Report(EWorkshopReportType eReport, string sMessage, BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_Report(): %1, %2, %3", eReport, sMessage, callback));
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.Report(eReport, sMessage, callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_Report()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_CancelReport(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_CancelReport(): %1", callback));
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.CancelReport(callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_CancelReport()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Called each frame by Addon Manager.
	void Internal_Update(float timeSlice)
	{	
		// Check if we are waiting for dependencies or revisions
		if (!GetRevisionsLoaded())
			TryLoadRevisions(false);
		//if (!GetDependenciesLoaded())
		//	TryLoadDependencies(false);		
		
		
		// Update all our current actions
		// If our own action are completed, we unref and detach them
		// We don't delete the actions so that users can still access them
		if (m_ActionDownload)
		{
			m_ActionDownload.Internal_Update();
			
			float newProgress;
			if (m_Item)
				newProgress = m_Item.GetProgress();
			if (newProgress != m_fPrevDownloadProgress)
			{
				/*
				// Too spammy
				#ifdef WORKSHOP_DEBUG
				_print(string.Format("OnChanged: Internal_Update: Download Progress Changed: %1", newProgress));
				#endif
				*/
				
				SetChanged();
				m_fPrevDownloadProgress = newProgress;
			}
			
			if (m_ActionDownload.IsCompleted())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download Is Completed");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				m_OnDownloadComplete.Invoke(this);
				SetChanged();
			}
			else if (m_ActionDownload.IsCanceled())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download was canceled, unregistering");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				SetChanged();
			}
			else if (m_ActionDownload.IsFailed())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download has failed, unregistering");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				SetChanged();
			}
		}
			
		if (m_ActionReport)
		{
			m_ActionReport.Internal_Update();
			
			if (m_ActionReport.IsCompleted())
			{
				m_ActionReport.Internal_Detach();
				m_ActionReport = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Report Action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, true);
			}
		}
		
		if (m_ActionCancelReport)
		{
			m_ActionCancelReport.Internal_Update();
			
			if (m_ActionCancelReport.IsCompleted())
			{
				m_ActionCancelReport.Internal_Detach();
				m_ActionCancelReport = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Cancel report Action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, false);
			}
		}
			
		if (m_ActionDependency)
		{
			m_ActionDependency.Internal_Update();
			
			if (m_ActionDependency.IsCompleted())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action is completed, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
				return;
			}
			else if (m_ActionDependency.IsCanceled())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action is canceled, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
			}
			else if (m_ActionDependency.IsFailed())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action has failed, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
			}
		}
		
		// Update the offline state
		bool offlineNew = GetOffline();
		if (offlineNew != m_bOffline)
		{
			m_OnOfflineStateChanged.Invoke(this, offlineNew);
		}
		m_bOffline = offlineNew;
		
		// Update the access state
		if (m_Item)
		{
			int accessState = m_Item.GetAccessState();
			if (accessState != m_iAccessState)
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Access state has changed");
				#endif			
				SetChanged();
				m_iAccessState = accessState;
			}
		}
		
		
		// Invoke changed script invoker
		if (m_bChanged)
		{
			
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("Invoking OnChanged: ID: %1, Name: %2", GetId(), GetName()));
			#endif
			
			m_OnChanged.Invoke(this);
			m_bChanged = false;
			
			// Invoke it for dependent addons too
			foreach (auto i : m_aDependent)
			{
				if (i)
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("Invoking OnChanged: ID: %1, Name: %2, for dependent: %3, %4", GetId(), GetName(), i.GetId(), i.GetName()));
					#endif
					i.m_OnChanged.Invoke(i);
				}
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Called by actions when some change happens
	void Internal_OnChanged()
	{
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when it can be unregistered bu addon manager
	//! We don't unregister items if they are offline or have any actions in progress, or if we have reported them (to avoid flushing item cache)
	bool Internal_GetCanBeUnregistered()
	{
		if (m_ActionDownload || m_ActionReport || m_ActionDependency || m_ActionCancelReport || GetOffline())
			return false;
		else
			return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when it can be unregistered bu addon manager
	WorkshopItem Internal_GetWorkshopItem()
	{
		return m_Item;
	}
	
	//-----------------------------------------------------------------------------------------------
	float Internal_GetDownloadProgress()
	{
		if (!m_Item)
			return 0;
		else return m_Item.GetProgress();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Creates a callback object, requests workshop API to load details
	void Internal_LoadDetails()
	{
		if (!m_CallbackAskDetails)
		{
			m_CallbackAskDetails = new SCR_WorkshopItemCallback_AskDetails(m_Item);
			m_CallbackAskDetails.m_OnGetAsset.Insert(Callback_AskDetails_OnGetAsset);
			//m_CallbackAskDetails.m_OnGetDependencyTree.Insert(Callback_AskDetails_OnGetDependencyTree); // Not needed any more - we load dependencies in another request.
			// m_CallbackAskDetails.m_OnGetAssetScenarios.Insert(Callback_AskDetails_OnGetScenarios); // Not needed any more
			m_CallbackAskDetails.m_OnTimeout.Insert(Callback_AskDetails_OnTimeout);
			m_CallbackAskDetails.m_OnError.Insert(Callback_AskDetails_OnError);
		}
		
		if (!m_bWaitingLoadDetails)
		{
			if (m_Item)
			{
				m_Item.AskDetail(m_CallbackAskDetails);			// Internally both methods perform the same request
				m_bWaitingLoadDetails = true;
			}
			else if (m_Dependency)
			{
				m_Dependency.LoadItem(m_CallbackAskDetails);
				m_bWaitingLoadDetails = true;
			}
		}
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void SetChanged()
	{
		m_bChanged = true;
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	
	
	
	
	
	
	//-----------------------------------------------------------------------------------------------
	//						C O N S T R U C T I O N
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	//! Private constructor - don't instantiate this yourself! Use SCR_AddonManager instead.
	private void SCR_WorkshopItem(WorkshopItem item, Dependency dependency)
	{
		m_Item = item;
		m_Dependency = dependency;
		
		if (m_Item)
		{
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("NEW for WorkshopItem: ID: %1, Name: %2", m_Item.Id(), m_Item.Name()));
			#endif
			
			UpdateStateFromWorkshopItem();
		}
		
		if(m_Dependency)
		{
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("NEW for Dependency: ID: %1, Name: %2", m_Dependency.GetID(), m_Dependency.GetName()));
			#endif
		}
		
		m_bOffline = GetOffline();
		
		#ifdef WORKSHOP_DEBUG
		m_sName = GetName();
		m_sId = GetId();
		#endif
	}
	
	//-----------------------------------------------------------------------------------------------
	void ~SCR_WorkshopItem()
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("DELETE for: ID: %1, Name: %2", this.GetId(), this.GetName()));
		#endif
		
		if (m_aDependencies)
		{
			foreach (SCR_WorkshopItem dependency : m_aDependencies)
			{
				if (dependency)
				{
					// Unregister this from dependency, so that it doesn't point back at a null
					dependency.UnregisterDependent(this);
				}
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void Internal_UpdateObjects(WorkshopItem item, Dependency dependency)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_UpdateObjects: %1, %2", item, dependency));
		#endif
		
		if (!m_Item && item)
		{
			m_Item = item;
			UpdateStateFromWorkshopItem();
		}
			
		// Alwasys update dependency - because it might be required to join a different server and thus dependency object
		// will be different!
		if (dependency)
			m_Dependency = dependency;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when the game checks addons after game start.
	void Internal_OnAddonsChecked()
	{
		UpdateStateFromWorkshopItem();
	}
	
	//-----------------------------------------------------------------------------------------------
	static SCR_WorkshopItem Internal_CreateFromWorkshopItem(WorkshopItem item)
	{
		SCR_WorkshopItem wrapper = new SCR_WorkshopItem(item, null);
		return wrapper;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	static SCR_WorkshopItem Internal_CreateFromDependency(Dependency dependency)
	{
		WorkshopItem cachedItem = dependency.GetCachedItem();
		#ifdef WORKSHOP_DEBUG
		_sprint(string.Format("Internal_CreateFromDependency(): cached Workshop Item was found"));
		#endif
		SCR_WorkshopItem wrapper = new SCR_WorkshopItem(cachedItem, dependency);
		return wrapper;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void UpdateStateFromWorkshopItem()
	{	
		if (m_Item)
		{
			m_bSubscribed = m_Item.IsSubscribed();
			m_bMyRating = m_Item.MyRating();
			m_bMyRatingSet = m_Item.IsRatingSet();
			m_bFavourite = m_Item.IsFavourite();
			
			if (m_Item.IsEnabled())
				m_Item.NotifyPlay();
		}
		
		TryLoadRevisions();
		TryLoadDependencies();
		TryLoadScenarios();
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static void _sprint(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1", str), logLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1 %2", this, str), logLevel);
	}
};
