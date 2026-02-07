// Mods callback
//------------------------------------------------------------------------------------------------
class SCR_BackendCallbackRoomMods : SCR_BackendCallback
{
	protected Room m_Room;
	
	//-----------------------------------------------------------------------------------------------
	Room GetRoom()
	{
		return m_Room;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SCR_BackendCallbackRoomMods(Room room)
	{
		m_Room = room;
	}
};

//-----------------------------------------------------------------------------------------------
void ScriptInvoker_RoomMods(Room room);
typedef func ScriptInvoker_RoomMods;

//-----------------------------------------------------------------------------------------------
void ScriptInvoker_RoomModsDependencies(Room room, array<ref SCR_WorkshopItem> dependencies);
typedef func ScriptInvoker_RoomModsDependencies;

//------------------------------------------------------------------------------------------------
//! This class will take care of handling all room mods & dependencies
//! Receiveing, sorting, downloading
class SCR_RoomModsManager
{
	protected Room m_Room;
	protected Dependency m_ScenarioDependency;
	
	// Rooms for caching 
	protected Room m_RoomLoaded;
	protected ref SCR_DownloadSequence m_DownloadSequence;
	
	protected int m_iLoadedModItems = 0;
	protected bool m_bModsLoaded = false;
	protected bool m_bModsMatching = false;
	protected bool m_bBlockedMods;
	protected bool m_bFailedModsLoading;
	protected ref array<ref SCR_WorkshopItem> m_aCollidingDependencies = {};
	
	// Arrays 
	protected ref array<ref SCR_WorkshopItem> m_aRoomItemsScripted = {};
	
	protected ref array<ref SCR_WorkshopItem> m_aItemsUpdated = {};
	protected ref array<ref SCR_WorkshopItem> m_aItemsToUpdate = {};
	
	// Callbacks 
	protected ref SCR_BackendCallbackRoomMods m_ModsCallback;
	protected ref SCR_OnlineServiceBackendCallbacks m_ScenarioCallback = new SCR_OnlineServiceBackendCallbacks();
	
	protected ref array<ref SCR_OnlineServiceBackendCallbacks> m_aModsCallbacks = {};
	
	protected float m_fModsSize = 0;
	
	//------------------------------------------------------------------------------------------------
	// Invokers
	//------------------------------------------------------------------------------------------------
	
	protected ref ScriptInvoker m_OnGetAllDependencies = new ScriptInvoker();
	
	protected ref ScriptInvoker m_OnGetScenario = new ScriptInvoker();
	
	protected ref ScriptInvokerBase<ScriptInvoker_RoomMods> m_OnModsFail;
	protected ref ScriptInvokerBase<ScriptInvoker_RoomModsDependencies> m_OnDependenciesLoadingPrevented;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnGetAllDependencies()
	{
		return m_OnGetAllDependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnGetScenario()
	{
		return m_OnGetScenario;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_RoomMods> GetOnModsFail()
	{
		if (!m_OnModsFail)
			m_OnModsFail = new ScriptInvokerBase<ScriptInvoker_RoomMods>();
		
		return m_OnModsFail;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_RoomModsDependencies> GetOnDependenciesLoadingPrevented()
	{
		if (!m_OnDependenciesLoadingPrevented)
			m_OnDependenciesLoadingPrevented = new ScriptInvokerBase<ScriptInvoker_RoomModsDependencies>();
		
		return m_OnDependenciesLoadingPrevented;
	}
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void DefaultSetup()
	{
		// Default for loading mods

		m_bModsLoaded = false;
		m_bModsMatching = false;
		m_bBlockedMods = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getting all mods for selected room 
	//! Checkign dependencies and loading workshop items of mods 
	void ReceiveRoomContentData(Room room)
	{
		if (!room)
			return;
		
		if (room != m_Room)
			m_Room = room;
		
		// Get mods and scenario
		ReceiveRoomMods(room); 
	}
	
	//------------------------------------------------------------------------------------------------
	void ReceiveRoomMods(notnull Room room)
	{	
		if (!room)
			return;
		
		if (room != m_Room)
			m_Room = room;
		
		array<Dependency> deps = {};  
		m_Room.GetItems(deps);
		
		m_bBlockedMods = false;
		m_bFailedModsLoading = false;
		m_aCollidingDependencies.Clear();
		
		// Load full list 
		m_ModsCallback = new SCR_BackendCallbackRoomMods(room);
		m_ModsCallback.GetEventOnResponse().Insert(OnRoomCallbackResponse);
		m_Room.LoadDownloadList(m_ModsCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	// Try to receive scenario data without looking for mods 
	void ReceiveRoomScenario(notnull Room room)
	{
		if (!room)
			return;
		
		if (room != m_Room)
			m_Room = room;
		
		// Check scenario cached
		m_ScenarioDependency = room.HostScenarioMod();
		
		// Handle online scenario
		if (m_ScenarioDependency)
		{
			if (!m_ScenarioDependency.GetCachedItem())
			{
				// Load uncached online scenario 
				m_ScenarioCallback.m_OnItem.Clear();
				m_ScenarioCallback.m_OnItem.Insert(OnScenarioModLoaded);
				m_ScenarioDependency.LoadItem(m_ScenarioCallback);
				return;
			}

			// Pass cached online scenario
			OnScenarioModLoaded();
			return;
		}
	
		// Pass local scenario - MissionWorkshopItem
		MissionWorkshopItem scenarioItem = room.HostScenario();
		OnScenarioModLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDownloadActions(notnull array<ref SCR_WorkshopItemActionDownload> downloads) 
	{ 
		foreach (SCR_WorkshopItem item : m_aItemsToUpdate)
		{
			downloads.Insert(item.GetDownloadAction());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invoke mods loading error 
	protected void OnLoadingModsFail(Room room)
	{
		// Don't notify fail if failed is not for current
		if (room != m_Room)
			return;
		
		m_bFailedModsLoading = true;
		
		if (m_OnModsFail)
			m_OnModsFail.Invoke(m_Room);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFailedModsLoading()
	{
		return m_bFailedModsLoading;
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Return only all updated dependecies from dependencies param
	//! Save all up to date to give out reference array 
	//! Save all up missing or outdate to out reference array 
	protected void UpdatedDependencies(notnull array<ref SCR_WorkshopItem> items)
	{	
		m_aItemsUpdated.Clear();
		m_aItemsToUpdate.Clear();
		
		m_aItemsUpdated = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.LOCAL_VERSION_MATCH_DEPENDENCY);
		m_aItemsToUpdate = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);
	} 
	
	//------------------------------------------------------------------------------------------------
	protected void AllItemsLoaded()
	{
		if (!m_Room)
			return;
		
		UpdatedDependencies(m_aRoomItemsScripted);

		// Pass list of updated & outdated mods
		if (m_OnGetAllDependencies)
		{
			m_OnGetAllDependencies.Invoke();
			
			m_bModsLoaded = true;
			m_bModsMatching = (m_aItemsToUpdate.IsEmpty());
			
			// Store room with loaded data 
			m_RoomLoaded = m_Room;
		}
		
		ReceiveRoomScenario(m_Room);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool AreAllModsLoaded(notnull Room room)
	{
		array<Dependency> items = {};
		room.AllItems(items);
		
		if (items.IsEmpty())
			return false;
		
		foreach (Dependency item : items)
		{
			if (!item.GetCachedItem())
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when sceario is loaded 
	protected void LoadScenario()
	{
		m_ScenarioCallback.m_OnItem.Clear();
		
		if (!m_Room)
			return;
					
		// Check scenario cached
		m_ScenarioDependency = m_Room.HostScenarioMod();
		MissionWorkshopItem mission = m_Room.HostScenario();
		
		if (m_ScenarioDependency)
		{
			// Handle online scenario
			if (!m_ScenarioDependency.GetCachedItem())
			{
				// Load workshop scenario 
				m_ScenarioCallback.m_OnScenarios.Insert(OnScenarioDetails);
				m_ScenarioDependency.LoadItem(m_ScenarioCallback);
			}
			else
			{
				OnScenarioDetails();
			}

		}
		else
		{
			// Pass local scenario - MissionWorkshopItem
			MissionWorkshopItem scenarioItem = m_Room.HostScenario();
			OnScenarioDetails();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Show restricted addons and return true if there are restricted addons in room content
	SCR_ReportedAddonsDialog DisplayRestrictedAddonsList()
	{
		/*
		if (m_DownloadSequence && m_DownloadSequence.GetRestrictedAddons())
			return m_DownloadSequence.ShowRestrictedDependenciesDialog();
		*/
		
		array<ref SCR_WorkshopItem> restrictedDependencies = SCR_AddonManager.SelectItemsBasic(m_aRoomItemsScripted, EWorkshopItemQuery.RESTRICTED);
		SCR_AddonListDialog addonsDialog = SCR_AddonListDialog.CreateRestrictedAddonsJoinServer(restrictedDependencies);
	
		// Handle cancel reports done 
		SCR_ReportedAddonsDialog reportedDialog = SCR_ReportedAddonsDialog.Cast(addonsDialog);
		if (reportedDialog)
			reportedDialog.GetOnAllReportsCanceled().Insert(OnAllReportsCanceled);
		
		return reportedDialog;
		
		// All can be downloaded
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when all reports from dialog are cancled to clear invoker actions and display download dialog 
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
		
		m_DownloadSequence.OnAllReportsCanceled(dialog);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mod containing scenario is loaded  
	protected void OnScenarioModLoaded()
	{
		// Remove callback action
		m_ScenarioCallback.m_OnItem.Remove(OnScenarioModLoaded);
		
		WorkshopItem itemScenario;
		
		// Load if scenario is missing 
		if (!m_ScenarioDependency)
		{
			LoadScenario();
			return;
		}
		
		// Check if scerio item was loaded 
		if (m_ScenarioDependency.GetCachedItem())
		{
			itemScenario = m_ScenarioDependency.GetCachedItem();
		}
		else
		{
			LoadScenario();
		}
		
		if (itemScenario)
		{
			// Load scenario details  
			m_ScenarioCallback.m_OnItem.Clear();
			m_ScenarioCallback.m_OnItem.Insert(LoadScenario);
			itemScenario.AskDetail(m_ScenarioCallback);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when scenario has got complete info 
	protected void OnScenarioDetails()
	{
		m_ScenarioCallback.m_OnScenarios.Remove(OnScenarioDetails);
		m_OnGetScenario.Invoke(m_ScenarioDependency);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRoomCallbackResponse(SCR_BackendCallbackRoomMods callback)
	{
		if (callback.GetResponseType() == EBackendCallbackResponse.SUCCESS)
		{
			// Load list success
			OnRoomFullList();
		}
		else
		{
			// Fail/timeout
			OnLoadingModsFail(callback.GetRoom());
		}
		
		callback.GetEventOnResponse().Remove(OnRoomCallbackResponse);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when room receive list of all mod dependencies 
	protected void OnRoomFullList()
	{
		if (!m_Room)
		{
			OnLoadingModsFail(null);
			return;
		}
		
		if (!m_Room.IsAuthorized())
		{
			OnLoadingModsFail(m_Room);
			return;
		}
		
		array<Dependency> deps = new array<Dependency>;
		m_Room.AllItems(deps);
		
		array<Dependency> depsItems = new array<Dependency>;
		
		// Clearup
		m_aRoomItemsScripted.Clear();
			
		// Skip loadig of content if none 
		if (deps.IsEmpty())
		{
			AllItemsLoaded();
			return;
		}
		
		SCR_AddonManager mngr = SCR_AddonManager.GetInstance();
		
		// Loaded dependencies as items
		foreach (Dependency dep : deps)
		{
			//ref SCR_WorkshopItem item = SCR_WorkshopItem.Internal_CreateFromDependency(dep);
			SCR_WorkshopItem item = mngr.Register(dep);
			item.SetItemTargetRevision(dep.GetRevision());
			m_aRoomItemsScripted.Insert(item);
		}
		
		// Clear previous download sequence reactions
		if (m_DownloadSequence)
		{
			m_DownloadSequence.GetOnReady().Remove(OnDownloadSequenceReady);
			m_DownloadSequence.GetOnError().Remove(OnDownloadSequenceError);
		}
		
		// Setup new download sequence
		InitDownloadSequence();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitDownloadSequence()
	{
		m_DownloadSequence = SCR_DownloadSequence.Create(m_aRoomItemsScripted, null, true);
		m_DownloadSequence.GetOnReady().Insert(OnDownloadSequenceReady);
		m_DownloadSequence.GetOnDependenciesLoadingPrevented().Insert(OnDependenciesLoadingPrevented);
		m_DownloadSequence.GetOnError().Insert(OnDownloadSequenceError);
		m_DownloadSequence.GetOnRestrictedDependency().Insert(OnDownloadSequenceRestrictedAddons);
		
		m_DownloadSequence.Init();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadSequenceReady(SCR_DownloadSequence sequence)
	{
		AllItemsLoaded();
		
		// Cleanup
		sequence.GetOnReady().Remove(OnDownloadSequenceReady);
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		sequence.GetOnError().Remove(OnDownloadSequenceError);
		sequence.GetOnRestrictedDependency().Remove(OnDownloadSequenceRestrictedAddons);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback when any of depeendencies are preventing loading or donwloading 
	//! Show dialog to cancel downloads blocking loading details
	protected void OnDependenciesLoadingPrevented(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> dependencies)
	{	
		m_aCollidingDependencies = dependencies;
		
		if (m_OnDependenciesLoadingPrevented)
			m_OnDependenciesLoadingPrevented.Invoke(m_Room, dependencies);
		
		// Cleanup
		sequence.GetOnReady().Remove(OnDownloadSequenceReady);
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		sequence.GetOnError().Remove(OnDownloadSequenceError);
		sequence.GetOnRestrictedDependency().Remove(OnDownloadSequenceRestrictedAddons);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetCollidingDependencies()
	{
		array<ref SCR_WorkshopItem> dependencies = {};
		
		foreach (SCR_WorkshopItem item : m_aCollidingDependencies)
		{
			dependencies.Insert(item);
		}
		
		return dependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadConfirm(SCR_ConfigurableDialogUi dialog)
	{
		SCR_DownloadManager.GetInstance().EndAllDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		InitDownloadSequence();
		
		dialog.m_OnConfirm.Remove(OnCancelDownloadConfirm);
		dialog.m_OnClose.Remove(OnCancelDownloadDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadSequenceError(SCR_DownloadSequence sequence)
	{
		// Cleanup
		sequence.GetOnReady().Remove(OnDownloadSequenceReady);
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		sequence.GetOnError().Remove(OnDownloadSequenceError);
		sequence.GetOnRestrictedDependency().Remove(OnDownloadSequenceRestrictedAddons);
	}
	
	protected void OnDownloadSequenceRestrictedAddons(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> items)
	{
		m_bBlockedMods = m_DownloadSequence.GetBlockedAddons();
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	string GetModListSizeString(array<ref SCR_WorkshopItem> items)
	{
		float size = SCR_ModHandlerLib.GetModListSize(items); 

		// Save size
		m_fModsSize = size;
		
		if (m_fModsSize <= 0)
			return string.Empty;
		
		// Size string
		string str = SCR_ByteFormat.GetReadableSize(size);
		
		return str;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get size with unit based on pach sizes from addons list
	string GetModListPatchSizeString(array<ref SCR_WorkshopItem> items)
	{
		float size;
		for (int i = 0, count = items.Count(); i < count; i++)
		{
			size += items[i].GetTargetRevisionPatchSize();
		}

		// Save size
		m_fModsSize = size;
		
		if (m_fModsSize <= 0)
			return string.Empty;
		
		// Size string
		return SCR_ByteFormat.GetReadableSize(size);
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasBlockedMods()
	{
		return m_bBlockedMods;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsScripted() 
	{ 
		return m_aRoomItemsScripted; 
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsUpdated()
	{ 
		return m_aItemsUpdated; 
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsToUpdate() 
	{ 
		array<ref SCR_WorkshopItem> items = {};
		
		foreach (SCR_WorkshopItem item : m_aItemsToUpdate)
		{
			items.Insert(item);
		}
		
		return items; 
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRoom(Room room) 
	{ 
		m_Room = room; 
	} 
	
	//------------------------------------------------------------------------------------------------
	bool GetModsLoaded() 
	{ 
		return m_bModsLoaded; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetModsMatching() 
	{ 
		return m_bModsMatching; 
	}
	
	//------------------------------------------------------------------------------------------------
	float GetModsSize() 
	{ 
		return m_fModsSize; 
	}
};