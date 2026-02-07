// Mods callback
//------------------------------------------------------------------------------------------------
class RoomModsCallback : SCR_OnlineServiceBackendCallbacks
{
	SCR_RoomModsManager m_ui;
	ref ScriptInvoker m_OnFullList = new ref ScriptInvoker;

	override void OnSuccess(int code)
	{
		super.OnSuccess(code);

		// Invoke full list on getting all server mods 
		if (code == EBackendRequest.EBREQ_WORKSHOP_GetDownloadList)
			m_OnFullList.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{
		#ifdef SB_DEBUG
		Print("ServerAddonsCallback - Error");
		#endif
		
		if (m_ui)
			m_ui.OnLoadingModsFail();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		#ifdef SB_DEBUG
		Print("ServerAddonsCallback - TimeOut");
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void RoomModsCallback(SCR_RoomModsManager ui)
	{
		m_ui = ui;
	}
};
// Mods callback

//------------------------------------------------------------------------------------------------
//! This class will take care of handling all room mods & dependencies
//! Receiveing, sorting, downloading
class SCR_RoomModsManager
{
	protected Room m_Room;
	protected WorkshopApi m_WorkshopApi;
	protected Dependency m_ScenarioDependency;
	
	// Rooms for caching 
	protected Room m_RoomLoaded;
	protected Room m_RoomOnDonwload;
	
	protected int m_iLoadedModItems = 0;
	protected bool m_bModsLoaded = false;
	protected bool m_bModsMatching = false;
	
	// Arrays 
	protected ref array<ref Dependency> m_aRoomDependecies = {};
	
	protected ref array<ref SCR_WorkshopItem> m_aRoomItemsScripted = {};
	
	protected ref array<ref SCR_WorkshopItem> m_aItemsUpdated = {};
	protected ref array<ref SCR_WorkshopItem> m_aItemsToUpdate = {};
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aDownloads = {};
	
	// Missions folder for in game scenarios 
	protected ref array<string> m_aMissionFolderPaths = {};
	protected ref array<ref SCR_MissionHeader> m_aMissionHeaders = {};
	protected string m_sWorldHeadersPattern = ".conf";
	
	// Invokers 
	ref ScriptInvoker m_OnGettingAllDependecies = new ref ScriptInvoker;
	ref ScriptInvoker m_OnModsFail = new ref ScriptInvoker;
	
	ref ScriptInvoker m_OnGettingScenario = new ref ScriptInvoker;
	
	ref ScriptInvoker m_OnModDownload = new ref ScriptInvoker;
	
	// Callbacks 
	protected ref RoomModsCallback m_ModsCallback = new RoomModsCallback(this);
	protected ref WorkshopCallback m_WorkshopCallback = new WorkshopCallback();
	protected ref SCR_OnlineServiceBackendCallbacks m_ScenarioCallback = new SCR_OnlineServiceBackendCallbacks();
	
	protected ref array<ModDownloadCallback> m_aDownloadCallbacks = {};
	protected ref array<ref SCR_OnlineServiceBackendCallbacks> m_aModsCallbacks = {};
	
	protected int m_iLoadedModItemsCount = 0;
	protected float m_fModsSize = 0;
	
	// String lenght constants 
	const int STRING_LENGHT_MOD_SIZE = -1;
	const int STRING_DEC_LENGHT_MOD_SIZE = 1;
	
	//------------------------------------------------------------------------------------------------
	protected void InialSetup()
	{
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop(); 
		
		// array cleanups
		ClearModArrays();

	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearModArrays()
	{
		m_aRoomDependecies.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Return only all updated dependecies from dependencies param
	//! Save all up to date to give out reference array 
	//! Save all up missing or outdate to out reference array 
	protected void UpdatedDependencies(array<ref SCR_WorkshopItem> items)
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
		
		array<Dependency> deps = new array<Dependency>;
		m_Room.AllItems(deps);
		m_aRoomItemsScripted.Clear();
		
		foreach (Dependency dep : deps)
		{
			// Register dependency 
			SCR_WorkshopItem item = SCR_AddonManager.GetInstance().Register(dep);
			
			// Save workshop item
			WorkshopItem cachedItem = dep.GetCachedItem();
			
			if (cachedItem)
				m_aRoomItemsScripted.Insert(item);
		}
		
		UpdatedDependencies(m_aRoomItemsScripted);

		// Pass list of updated & outdated mods
		if (m_OnGettingAllDependecies)
		{
			m_OnGettingAllDependecies.Invoke();
			
			m_bModsLoaded = true;
			m_bModsMatching = (m_aItemsToUpdate.IsEmpty());
			
			// Store room with loaded data 
			m_RoomLoaded = m_Room;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this on mod image data loaded 
	protected void OnImageDownload()
	{
		// Is still downloading check 
		foreach (Dependency dep : m_aRoomDependecies)
		{
			// Check item 
			WorkshopItem item = dep.GetCachedItem();
			if (!item)
				return;

			// Don't remove callback until image is loaded 
			int stateFlags = item.GetStateFlags();
			
			//TODO replace obsolete code
			//if (stateFlags & EWorkshopItemState.EWSTATE_DOWNLOADING_IMAGE)
			//	return;
		}
		
		// Remove callback action 
		m_ModsCallback.m_OnImage.Remove(OnImageDownload);
	}
	
	//------------------------------------------------------------------------------------------------
	void DefaultSetup()
	{
		// Default for loading mods
		m_ModsCallback.m_OnItem.Clear();
		m_iLoadedModItemsCount = 0;
		
		m_bModsLoaded = false;
		m_bModsMatching = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getting all mods for selected room 
	//! Checkign dependencies and loading workshop items of mods 
	void ReceiveRoomContentData(Room room)
	{
		m_Room = room;
		if (!m_Room)
			return;
		
		// Setup
		WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
		workshopApi.SetCallback(m_WorkshopCallback);
		
		ReceiveRoomMods(room);
		
		// Get room scenario 
		ReceiveRoomScenario(room);
	}
	
	//------------------------------------------------------------------------------------------------
	void ReceiveRoomMods(notnull Room room)
	{	
		if (!room)
			return;
		
		if (room != m_Room)
			m_Room = room;
		
		// Skip if room mods data is loaded 
		if (room == m_RoomLoaded || AllModsLoaded(room))
		{
			AddonsFullList();
			UpdatedDependencies(m_aRoomItemsScripted);
			
			m_bModsLoaded = true;
			m_OnGettingAllDependecies.Invoke();
			return;
		}
		
		// Clear arrays 
		ClearModArrays();
		
		// Load full list 
		m_ModsCallback.m_OnFullList.Insert(AddonsFullList);
		m_Room.LoadDownloadList(m_ModsCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool AllModsLoaded(notnull Room room)
	{
		array<Dependency> items = {};
		room.AllItems(items);
		
		if (items.IsEmpty())
			return false;
		
		for (int i = 0, count = items.Count(); i < count; i++)
		{
			if (!items[i].GetCachedItem())
				return false;
		}
		
		return true;
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
				m_ScenarioCallback.m_OnItem.Insert(ScenarioModLoaded);
				m_ScenarioDependency.LoadItem(m_ScenarioCallback);
				return;
			}

			// Pass cached online scenario
			m_OnGettingScenario.Invoke(m_ScenarioDependency);
			return;
		}
	
		// Pass local scenario - MissionWorkshopItem
		MissionWorkshopItem scenarioItem = room.HostScenario();
		m_OnGettingScenario.Invoke(m_ScenarioDependency);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mod containing scenario is loaded  
	protected void ScenarioModLoaded()
	{
		// Remove callback action
		m_ScenarioCallback.m_OnItem.Remove(ScenarioModLoaded);
		
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
			LoadScenario();
		
		if (itemScenario)
		{
			// Load scenario details  
			m_ScenarioCallback.m_OnItem.Clear();
			m_ScenarioCallback.m_OnItem.Insert(LoadScenario);
			itemScenario.AskDetail(m_ScenarioCallback);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when sceario is loaded 
	void LoadScenario()
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
	//! Call this when scenario has got complete info 
	protected void OnScenarioDetails()
	{
		m_ScenarioCallback.m_OnScenarios.Remove(OnScenarioDetails);
		m_OnGettingScenario.Invoke(m_ScenarioDependency);
	}
	
	protected int m_iDebugModCount = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Call this when room receive list of all mod dependencies 
	protected void AddonsFullList()
	{
		m_ModsCallback.m_OnFullList.Remove(AddonsFullList);
		
		m_iDebugModCount = 0;
		
		if (!m_Room)
			return;
		
		array<Dependency> deps = new array<Dependency>;
		m_Room.AllItems(deps);
		
		m_iLoadedModItemsCount = 0; 
		
		// Save this in to strong ref dependency array and register scripted items 
		m_aRoomDependecies.Clear();
		m_aRoomItemsScripted.Clear();
		
		foreach (Dependency dep : deps)
		{
			m_aRoomDependecies.Insert(dep);
		}
		
		// Clear mod callbacks 
		m_aModsCallbacks.Clear();
		
		//Print("[SCR_RoomModsManager.AddonsFullList] dependencies for: " + m_Room.Name());
		
		// Go through mods dependecies if their items were loaded 
		foreach (Dependency dep : m_aRoomDependecies)
		{
			// Is item loaded 
			WorkshopItem item = dep.GetCachedItem();
			//Print(item);
			
			if (item)
			{
				m_iLoadedModItemsCount++;
				continue;
			}
			
			// Load item if is not loaded 
			
			// Set invoker actions 
			SCR_OnlineServiceBackendCallbacks modCallback = new SCR_OnlineServiceBackendCallbacks;
			modCallback.m_OnItem.Insert(OnLoadingModItem);
			
			// Add callbacks 
			m_aModsCallbacks.Insert(modCallback);
			
			// Load ite
			dep.LoadItem(modCallback);
		}
		
		// Is loading done 
		if (m_iLoadedModItemsCount >= m_aRoomDependecies.Count())
			AllItemsLoaded();
		else 
		{
			// Add loading check callback 
			/*m_ModsCallback.m_OnItem.Clear();
			m_ModsCallback.m_OnItem.Insert(OnLoadingModItem);
			m_ModsCallback.m_OnImage.Clear();
			m_ModsCallback.m_OnImage.Insert(OnImageDownload);*/
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this callback when mod item finished loading
	protected void OnLoadingModItem()
	{
		m_iDebugModCount++;
		//Print("Loading mod success: " + m_iDebugModCount);
		
		// Empty room dependencies check - safe check, sholdn't be happenning with proper use 
		if (m_aRoomDependecies.IsEmpty())
		{
			#ifdef SB_DEBUG
			Print("Items loaded was called, but items list is empty! - of: " + m_Room.Name(), LogLevel.ERROR);
			#endif
			return;
		}	

		// Check loaded items 
		if (m_aRoomDependecies.Count() < m_iLoadedModItemsCount)
			return;
		
		/*Dependency dep = m_aRoomDependecies[m_iLoadedModItemsCount];
		
		if (!dep)
			return;

		if (dep.GetCachedItem())
		{
			// Register item as loaded 
			m_iLoadedModItemsCount++;
			//Print("		Loaded mods: " + m_iLoadedModItemsCount);
		}
		else 
		{
			// Add to list for items to be loaded 
			m_aRoomDependecies[m_iLoadedModItemsCount].LoadItem(m_ModsCallback);
		}*/
		
		m_iLoadedModItemsCount++;
		
		// TODO: remove when done with debug 
		if (m_iDebugModCount == m_aRoomDependecies.Count() && m_iLoadedModItemsCount < m_aRoomDependecies.Count())
		{
			//Print("Not all mods loaded");
		}
		
		// Is loading done 
		if (m_iLoadedModItemsCount >= m_aRoomDependecies.Count())
		{
			AllItemsLoaded();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invoke mods loading error 
	void OnLoadingModsFail()
	{
		m_OnModsFail.Invoke(false);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetModListSizeString(array<ref SCR_WorkshopItem> item)
	{
		float size = SCR_ModHandlerLib.GetModListSize(item); 
		//float size = SCR_DownloadManager.GetModListSize();
		
		// Save size
		m_fModsSize = size;
		
		if (m_fModsSize <= 0)
			return string.Empty;
		
		// Size string
		string str = SCR_ByteFormat.GetReadableSize(size);
		//str = size.ToString(STRING_LENGHT_MOD_SIZE, STRING_DEC_LENGHT_MOD_SIZE) + str;
		
		return str;
	}
	
	// Subscribing and downloading
	
	ref ScriptInvoker m_OnDataDownload = new ref ScriptInvoker;
	protected float m_fDownloadProgress = 0;
	protected int m_iDownloadedCount = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Subscribe to all outdated and missing mods 
	void UpdateMods()
	{
		// Default setup 
		m_fDownloadProgress = 0; 
		m_iDownloadedCount = 0;
		m_aDownloadCallbacks.Clear();
		m_aDownloads.Clear();

		// Mods data size - TODO@wernerjak - use download manager for this 
		m_fModsSize = SCR_ModHandlerLib.GetModListSize(m_aItemsToUpdate);
		
		// Register mods and start download 
		foreach (SCR_WorkshopItem mod : m_aItemsToUpdate)
		{
			// Get vresion 
			Dependency dependency = mod.GetDependency();
			string version = dependency.GetVersion();
			
			// Start download 
			SCR_WorkshopItemActionDownload download = mod.Download(version);
			download.Activate();
			
			// Subscribe on download finish 
			mod.m_OnDownloadComplete.Insert(OnModDonwloadComplete);
			
			// Add download action 
			m_aDownloads.Insert(mod.GetDownloadAction());
		}
		
		// Store server where mods are downloaded 
		m_RoomOnDonwload = m_Room;
	}
	
	//------------------------------------------------------------------------------------------------
	void StopModDownload()
	{
		// TODO@wernerjak - proper download canceling 
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDownloadActions(array<ref SCR_WorkshopItemActionDownload> downloads) { m_aDownloads = downloads; }
	
	//------------------------------------------------------------------------------------------------
	//! Progress of mod to download
	float ModDownloadProggress()
	{
		return SCR_DownloadManager.GetDownloadActionsProgress(m_aDownloads);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDownloadingFinished()
	{
		int itemsC = m_aItemsToUpdate.Count();
		
		bool finished = (m_iDownloadedCount == itemsC);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mod download is finished
	//! Progress download state
	protected void OnModDonwloadComplete(SCR_WorkshopItem item)
	{
		item.m_OnDownloadComplete.Remove(OnModDonwloadComplete);
		m_iDownloadedCount++;
		
		m_OnModDownload.Invoke(item, m_aItemsToUpdate.Count(), m_iDownloadedCount);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RoomModsManager()
	{
		InialSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RoomModsManager()
	{
		ClearModArrays();
	}
	
	// API
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsScripted() { return m_aRoomItemsScripted; }
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsUpdated() { return m_aItemsUpdated; }
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsToUpdate() { return m_aItemsToUpdate; }
	
	//------------------------------------------------------------------------------------------------
	void SetRoom(Room room) { m_Room = room; } 
	
	//------------------------------------------------------------------------------------------------
	void SetRoomDependencies(array<ref Dependency> aDependencies) { m_aRoomDependecies = aDependencies; }
	
	//------------------------------------------------------------------------------------------------
	bool GetModsLoaded() { return m_bModsLoaded; }
	
	//------------------------------------------------------------------------------------------------
	bool GetModsMatching() { return m_bModsMatching; }
	
	//------------------------------------------------------------------------------------------------
	float GetModsSize() { return m_fModsSize; }
	
	// Mods counts 
	//------------------------------------------------------------------------------------------------
	int GetAllModsCount() { return m_aRoomDependecies.Count(); }
};

//------------------------------------------------------------------------------------------------
class ModDownloadCallback : SCR_OnlineServiceBackendCallbacks
{	
	protected Dependency m_Dependecy;
	protected float m_fLastProgress = 0;
	
	ref ScriptInvoker m_OnDataReceive = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);	
		
		// Download 
		if (code == EBackendRequest.EBREQ_WORKSHOP_DownloadAsset)
		{
			int size = m_Dependecy.TotalFileSize();
			float progress = m_Dependecy.GetCachedItem().GetProgress();
			 
			m_OnDataReceive.Invoke(this, size, progress);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ModDownloadCallback(Dependency dependency)
	{
		m_Dependecy = dependency;
		m_fLastProgress = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	Dependency GetDependency() { return m_Dependecy; }
	
	//------------------------------------------------------------------------------------------------
	float GetLastProgress() { return m_fLastProgress; }
	
	//------------------------------------------------------------------------------------------------
	void SetLastProgress(float progress) { m_fLastProgress = progress; }
};