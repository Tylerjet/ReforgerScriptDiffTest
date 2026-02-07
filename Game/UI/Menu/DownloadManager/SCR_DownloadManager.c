/*
This entity is required for SCR_DownloadManager_Dialog to work.

It performs tracking of current downloads, while the Dialog class just visualizes them when opened.

!!! This entity relies on SCR_AddonManager, which also must be placed into the world.
*/

[EntityEditorProps(category: "", description: "Entity of the download manager. Most likely only needed in the main menu world.")]
class SCR_DownloadManagerClass: GenericEntityClass
{
};

class SCR_DownloadManager : GenericEntity
{
	//! Script invoker for new downloads
	//! ALways gets called after the new download is registered in the download manager.
	ref ScriptInvoker m_OnNewDownload = new ScriptInvoker; // (SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	
	protected static SCR_DownloadManager s_Instance;
	
	// All download actions
	protected ref array<ref SCR_DownloadManager_Entry> m_aDownloadActions = new array<ref SCR_DownloadManager_Entry>;
	
	// Download queue - it gets empty once all active downloads are complete, but keeps added up if new downloads are started
	// While previous are in progress.
	// This is mainly used by the panel.
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aDownloadQueue = new array<ref SCR_WorkshopItemActionDownload>;
	protected int m_iQueueDownloadsCompleted; // Amount of completed downloads in the queue
	
	// Bool to pause all downloads
	protected bool m_bDownloadsPaused;
	
	//-----------------------------------------------------------------------------------------------
	// 				P U B L I C   A P I 
	//-----------------------------------------------------------------------------------------------
	
	
	//-----------------------------------------------------------------------------------------------
	static SCR_DownloadManager GetInstance()
	{
		return s_Instance;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns an array of all downloads regardless of their state
	void GetAllDownloads(array<ref SCR_DownloadManager_Entry> downloads)
	{
		downloads.Clear();
		
		foreach(auto dl : m_aDownloadActions)
			downloads.Insert(dl);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Might get delayed by a frame! Just use it for UI.
	void GetDownloadQueueState(out int nCompleted, out int nTotal)
	{
		nTotal = m_aDownloadQueue.Count();
		nCompleted = m_iQueueDownloadsCompleted;
	}
	
	//-----------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItemActionDownload> GetDownloadQueue()
	{
		array<ref SCR_WorkshopItemActionDownload> downloads = {};
		
		if (!m_aDownloadQueue)
			return downloads;
		
		foreach (auto i : m_aDownloadQueue)
			downloads.Insert(i);
		
		return downloads;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Return item action for given workshop item 
	SCR_WorkshopItemActionDownload GetActionOfItem(SCR_WorkshopItem item)
	{
		foreach (SCR_DownloadManager_Entry entry : m_aDownloadActions)
		{
			if (entry.m_Item == item)
				return entry.m_Action;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------
	// Paused or resumes all downloads globally.
	void SetDownloadsPaused(bool pause)
	{
		if (pause == m_bDownloadsPaused)
			return;
		
		m_bDownloadsPaused = pause;
		
		if (pause)
		{
			// Pause all download actions
			foreach (SCR_WorkshopItemActionDownload action : m_aDownloadQueue)
			{
				if (action.IsActive())
					action.Pause();
			}
		}
		else
		{
			// Resume or activate all download actions
			foreach (SCR_WorkshopItemActionDownload action : m_aDownloadQueue)
			{
				if (action.IsPaused())
					action.Resume();
				else if (action.IsInactive())
					action.Activate();
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetDownloadsPaused()
	{
		return m_bDownloadsPaused;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Ends all downloads in a safe manner.
	//! It is meant to be caused when we start a scenario or exit the game.
	//! The function should handle the termination of all downloads in safest way currently possible.
	void EndAllDownloads()
	{
		foreach (SCR_WorkshopItemActionDownload a : m_aDownloadQueue)
		{
			a.Cancel();
		}
	}
	
	
	// --- Helper functions for generic download functionality ---
	
	//-----------------------------------------------------------------------------------------------
	//! Checks if we need to start a new download of some addon
	//! Compared to just checking if it's offline, it is more contextual
	//! Because it also checks current download
	static bool IsLatestDownloadRequired(SCR_WorkshopItem item)
	{
		if (!item.GetOffline())
			return true;
		
		string latestVersion = item.GetLatestVersion();
		
		bool downloading, paused;
		string targetVersion;
		float progress;
		item.GetDownloadState(downloading, paused, progress, targetVersion);
		
		if (!downloading)
		{
			string currentVersion = item.GetCurrentLocalVersion();
			return currentVersion != latestVersion;
		}
		else
		{
			// We are already downloading this, but what versoin?
			if (latestVersion == targetVersion)
				return false; // Already downloading this version, no need to start a new download
			else
				return true;	// Downloading a different version, we must download another one
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	static void SelectAddonsForLatestDownload(array<ref SCR_WorkshopItem> arrayIn, array<ref SCR_WorkshopItem> arrayOut)
	{
		foreach (auto i : arrayIn)
		{
			if (IsLatestDownloadRequired(i))
				arrayOut.Insert(i);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Creates new download actions for downloading latest version of main item and all its dependencies.
	//! The actions are started if downloads are not paused in the download manager (GetDownloadsPaused())
	//! If the GetDownloadsPaused() is true, the actions are created but not activated.
	array<ref SCR_WorkshopItemAction> DownloadLatestWithDependencies(notnull SCR_WorkshopItem mainItem, bool downloadMainItem, array<ref SCR_WorkshopItem> dependencies)
	{
		array<ref SCR_WorkshopItemAction> actions = {};
		
		if (IsLatestDownloadRequired(mainItem) && downloadMainItem)
		{
			auto actionMain = mainItem.DownloadLatestVersion();
			if (actionMain)
			{
				actions.Insert(actionMain);
				if (!m_bDownloadsPaused)
					actionMain.Activate();
			}
		}
		
		if (dependencies)
		{
			if (dependencies.Count() > 0)
			{
				auto actionDependencies = mainItem.DownloadDependenciesLatest(dependencies);
				if (actionDependencies)
				{
					if (!m_bDownloadsPaused)
						actionDependencies.Activate();
					
					actions.Insert(actionDependencies);
				}
			}
		}
		
		return actions;
	}
	
	//-----------------------------------------------------------------------------------------------
	static float GetTotalSizeBytes(array<ref SCR_WorkshopItem> arrayIn, SCR_WorkshopItem extraItem = null)
	{
		float sizeOut = 0;
		
		foreach (auto i : arrayIn)
		{
			float s = i.GetSizeBytes();
			sizeOut += s;
		}
		
		if (extraItem)
		{
			float s = extraItem.GetSizeBytes();
			sizeOut += s;
		}
		
		return sizeOut;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns overall progress of all download actions, from 0 to 1
	static float GetDownloadActionsProgress(array<ref SCR_WorkshopItemActionDownload> actions)
	{
		float totalSizeBytes = 0;
		float totalBytesDownloaded = 0;
		foreach (auto dl : actions)
		{
			float dlsize = dl.GetSizeBytes();
			totalSizeBytes += dlsize;
			totalBytesDownloaded += dl.GetProgress() * dlsize;
		}
		
		if (totalSizeBytes == 0) // Don't divide by 0
			return 0;
		
		float progress = totalBytesDownloaded / totalSizeBytes;
		return progress;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns overall download progress for all actions of a workshop item
	static float GetItemDownloadActionsProgress(SCR_WorkshopItem item)
	{
		// Create an array of all download actions started from this tile
		// Get their aggregated progress
		
		auto actionThisItem = item.GetDownloadAction();					// Action for downloading this item
		auto actionDependencies = item.GetDependencyCompositeAction();	// Action for downloading dependencies
		
		array<ref SCR_WorkshopItemAction> allActions;
		if (actionDependencies)
			allActions = actionDependencies.GetActions();
		else
			allActions = new array<ref SCR_WorkshopItemAction>;
		
		if (actionThisItem)
			allActions.Insert(actionThisItem);
		
		// Cast all actions to download actions...
		auto allDownloadActions = new array<ref SCR_WorkshopItemActionDownload>;
		foreach (auto a : allActions)
		{
			auto downloadAction = SCR_WorkshopItemActionDownload.Cast(a);
			if (downloadAction)
				allDownloadActions.Insert(downloadAction);
		}
		
		float progress = SCR_DownloadManager.GetDownloadActionsProgress(allDownloadActions);
		
		return progress;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	

	
	
	//-----------------------------------------------------------------------------------------------
	// 					P R O T E C T E D   /   P R I V A T E
	//-----------------------------------------------------------------------------------------------
	
	
	
	//-----------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Remove failed or canceled actions from the download queue
		m_iQueueDownloadsCompleted = 0;
		for (int i = m_aDownloadQueue.Count() - 1; i >= 0; i--)
		{
			SCR_WorkshopItemActionDownload action = m_aDownloadQueue[i];
			
			if (action.IsCanceled() || action.IsFailed())
				m_aDownloadQueue.Remove(i);
			else
			{
				if (action.IsCompleted())
					m_iQueueDownloadsCompleted++;
			}
		}
		
		// If all are completed, clear the queue entirely
		// And reset 'all downloads paused' state.
		if (m_iQueueDownloadsCompleted == m_aDownloadQueue.Count())
		{
			m_aDownloadQueue.Clear();
			m_bDownloadsPaused = false;
		}
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();
		
		if (!addonManager)
		{
			Print("SCR_DownloadManager_Entity: SCR_AddonManager was not found. It must be placed in the world for download manager to work.", LogLevel.ERROR);
			return;
		}
		
		addonManager.m_OnNewDownload.Insert(Callback_OnNewDownload);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Gets called when a new download action is created.
	//! !!! Action creation doesn't mean start of a download. At this point the action is created but
	//! it is not activated yet.
	protected void Callback_OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Callback_OnNewDownloadStarted: %1, %2", item.GetName(), action.GetTargetVersion()));
		#endif
		
		// Create a new entry
		SCR_DownloadManager_Entry entry = new SCR_DownloadManager_Entry(item, action);
		m_aDownloadActions.Insert(entry);
		
		// Add the action to the queue
		m_aDownloadQueue.Insert(action);
		
		m_OnNewDownload.Invoke(item, action);
		
		action.m_OnCompleted.Insert(Callback_OnDownloadCompleted);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called from aciton when a download is completed
	protected void Callback_OnDownloadCompleted(SCR_WorkshopItemActionDownload action)
	{
		// Check user's settings if we need to enable the addon automatically
		// Ignore this if it was an update of an addon
		SCR_WorkshopSettings settings = SCR_WorkshopSettings.Get();
		if (settings.m_bAutoEnableDownloadedAddons && !action.GetUpdate())
		{	
			SCR_WorkshopItem item = action.GetWorkshopItem();
			
			if (!item)
				return;
			
			item.SetEnabled(true);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	private void SCR_DownloadManager(IEntitySource src, IEntity parent)
	{
		SetEventMask( EntityEvent.FRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
		
		s_Instance = this;
	}
	
	//-----------------------------------------------------------------------------------------------
	private void ~SCR_DownloadManager()
	{
		s_Instance = null;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	
	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_DownloadManager] %1 %2", this, str), logLevel);
	}	
};


//------------------------------------------------------------------------------------------------
//! Helper class to store current downloads and their attributes.
class SCR_DownloadManager_Entry
{
	ref SCR_WorkshopItem m_Item;					// We hold strong refs to both item and action!
	ref SCR_WorkshopItemActionDownload m_Action;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DownloadManager_Entry(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		m_Item = item;
		m_Action = action;
	}
};
