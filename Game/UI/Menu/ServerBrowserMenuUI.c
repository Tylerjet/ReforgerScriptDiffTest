/*
Main class for server browser menu handling 
Handles cooperation of varios components for server searching, joning, mods handling, etc.
*/
//------------------------------------------------------------------------------------------------
class ServerBrowserMenuUI: MenuRootBase
{	
	const ResourceName CONFIG_DIALOGS_ERROR = "{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf";
	
	// Server list feedback strings
	const string TAG_DETAILS_FALLBACK_SEACHING = "Searching";
	const string TAG_DETAILS_FALLBACK_EMPTY = "Empty";
	const string TAG_MESSAGE_SEARCHING = "SEARCHING";
	
	const int SERVER_LIST_VIEW_SIZE = 32;
	const int ROOM_CONTENT_LOAD_DELAY = 500;
	const int ROOM_REFRESH_RATE = 10 * 1000;
	const int ROOM_REFRESH_WAIT_DELAY = 100;
	const int BACKEND_CHECK_TIMEOUT = 30 * 1000;
	
	protected bool m_bWaitForRefresh = false;
	
	// Widget class reference
	protected ref ServerBrowserMenuWidgets m_Widgets = new ref ServerBrowserMenuWidgets;

	// Server list handling 
	protected SCR_ServerBrowserEntryComponent m_EntryInteractible = null;
	protected SCR_ServerBrowserEntryComponent m_ServerEntryFocused = null;
	protected SCR_ServerBrowserEntryComponent m_ServerEntryLastFocused = null;
	protected ref array<SCR_ServerBrowserEntryComponent> m_aRoomEntryList = {};
	
	protected Room m_LastFocusedRoom = null;
	protected Room m_LastLoadedRoom = null;
	
	// Widget handlers 
	protected SCR_TabViewComponent m_TabView;
	protected SCR_SortHeaderComponent m_SortBar;
	protected SCR_EditBoxComponent m_SearchEditBox;
	protected SCR_FilterPanelComponent m_FilterPanel;
	protected SCR_PooledServerListComponent m_ScrollableList;
	protected SCR_ServerScenarioDetailsPanelComponent m_ServerScenarioDetails;
	
	protected SCR_NavigationButtonComponent m_BtnJoin;
	protected SCR_NavigationButtonComponent m_BtnDetails;
	protected SCR_NavigationButtonComponent m_BtnFavorite;
	
	// States 
	protected Widget m_wFirstServerEntry;
	protected ESBWidgetFocus m_iFocusedWidgetState = ESBWidgetFocus.SERVER_LIST;
	
	protected static string m_sErrorMessage = "";
	protected static string m_sErrorMessageGroup = "";
	protected static string m_sErrorMessageDetail = "";
	
	// Lobby Rooms handling
	protected Room m_JoiningRoom = null;
	protected ref array<Room> m_aRooms = {};
	protected ref array<Room> m_aDirectFoundRooms = {};
	
	protected WorkshopApi m_WorkshopApi; 
	protected ClientLobbyApi m_Lobby; 
	protected ref ServerDetailsMenuUI m_DetailsDialog = null;
		
	// Search callbacks
	//ref OnSearchServers m_SearchCallback = new OnSearchServers;
	protected ref array<ref ServerBrowserCallback> m_aSearchCallbacks = {};
	protected ref ServerBrowserCallback m_CallbackLastSearch = null;
	protected ref ServerBrowserCallback m_CallbackScroll = new ServerBrowserCallback();
	
	protected ref OnDirectJoinCallback m_CallbackSearchTarget = new OnDirectJoinCallback();
	
	// Room data refresh
	protected ref ServerBrowserCallback m_CallbackAutoRefresh = new ServerBrowserCallback();
	protected bool m_bRoomsLoaded = false;
	protected bool m_bFirstRoomLoad = true;
	
	// Joining 
	protected ref ServerBrowserCallback m_CallbackJoin = new ref ServerBrowserCallback();
	protected ref SCR_ServerBrowserDialogManager m_Dialogs = new SCR_ServerBrowserDialogManager();
	
	protected ref OnJoinRoomSB m_CallbackPasswordCheck = new OnJoinRoomSB();

	// Filter parameters 
	protected ref FilteredServerParams m_ParamsFilter = new FilteredServerParams();
	protected ref FilteredServerParams m_DirectJoinParams = new FilteredServerParams();
	
	// Server mod content 
	protected ref SCR_RoomModsManager m_ModsManager = new SCR_RoomModsManager();
	
	// Dependecies arrays for pasing updated & outdated server mod dependecies 
	protected MissionWorkshopItem m_RoomScenario;
	
	protected ref array<Dependency> m_aServerDependencies = {};
	
	// Message components 
	protected SCR_SimpleMessageComponent m_SimpleMessageWrap;
	protected SCR_SimpleMessageComponent m_SimpleMessageList;
	
	// Connecting dialog handling 
	protected bool m_bServerItemsLoaded = false;
	protected bool m_bConnectMenuOpened = false;
	protected bool m_bServerBrowserReopened = false;
	protected bool m_bIsScenarioLoaded = false;
	
	protected bool m_bIsWaitingForBackend = true;
	
	// Reconnecting to last played server 
	protected ref ServerBrowserCallback m_CallbackSearchPreviousRoom = new ServerBrowserCallback();
	protected ref SCR_GetRoomsIds m_SearchIds = new SCR_GetRoomsIds();
	protected string m_sLastRoomId = string.Empty;
	protected Room m_RejoinRoom = null;
	
	// Script invokers 
	ref ScriptInvoker m_OnAutoRefresh = new ScriptInvoker;
	ref ScriptInvoker m_OnDependeciesLoad = new ScriptInvoker;
	ref ScriptInvoker m_OnScenarioLoad = new ScriptInvoker;
	ref ScriptInvoker m_OnEntryFocus = new ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	// Override menu functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Opening menu - do server browser setup - TODO@wernerjak - cleanup open
	override void OnMenuOpen()
	{
		if (!m_ParamsFilter)
			m_ParamsFilter = new FilteredServerParams();
		
		// Workshop setup
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop(); 
		m_Lobby = GetGame().GetBackendApi().GetClientLobby();
		
		m_Lobby.SetViewSize(SERVER_LIST_VIEW_SIZE);
		
		// Items preloading 
		if (m_WorkshopApi.NeedScan())
			m_WorkshopApi.ScanOfflineItems();
		
		// Mods manager 
		if (m_ModsManager)
			m_ModsManager.m_OnModsFail.Insert(OdModListFail);
		
		// Find widgets 
		m_Widgets.FindAllWidgets(GetRootWidget());
		
		// Accessing handlers 	
		SetupHandlers();
		SetupCallbacks();
		
		SetupParams(m_Lobby);
		
		m_iFocusedWidgetState = ESBWidgetFocus.SERVER_LIST;
		
		RoomHandlingButtonsEnabling(false, false);
		
		// Setup list and message 
		Messages_ShowMessage(TAG_MESSAGE_SEARCHING, true);
		
		if (m_ScrollableList)
		{
			m_ScrollableList.ShowEmptyRooms();
		}
		
		// Setup debug menu 
		
		// Setup connection attempt timeout 
		GetGame().GetCallqueue().CallLater(ConnectionTimeout, BACKEND_CHECK_TIMEOUT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		// Show last server
		if (!m_sErrorMessage.IsEmpty())
			DisplayKick();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Closing menu - clearing server browser data 
	override void OnMenuClose()
	{	
		if (m_DetailsDialog)
			m_DetailsDialog.Close();
		
		// Clearing handlers 
		if (m_TabView)
			m_TabView.m_OnChanged.Remove(OnTabViewSwitch);
		
		// Store filter pamarameters
		m_Lobby.StoreParams();
		
		// Remove callbacks 
		m_Lobby.SetRefreshCallback(null);

		m_ScrollableList.m_OnSetPage.Remove(CallOnServerListSetPage);
		m_CallbackAutoRefresh.m_OnSuccess.Clear();
		
		ClearConnectionTimeoutWaiting();
	}

	//------------------------------------------------------------------------------------------------
	//! Updating menu - continuous menu hanling 
	override protected void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		// On server reopen 
		if (m_bServerBrowserReopened)
		{
			RefocusEntry();
			m_bServerBrowserReopened = false;
		}
		
		// Run actions after backend initialization
		if (m_bIsWaitingForBackend)	
			WaitingForRunningBackend();
		
		// Testing scrollable list update
		if (m_ScrollableList)
			m_ScrollableList.UpdateScroll();
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Service check
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Start looking for servers once backend is runnig 
	protected void WaitingForRunningBackend()
	{		
		bool backendActive = GetGame().GetBackendApi().IsActive();
		if (!backendActive)
			return;
		
		ClearConnectionTimeoutWaiting();
		
		// Call room search or find last server 
		string lastId = m_Lobby.GetPreviousRoomId();
		
		// Is there last server and reconnect is enabled
		bool reconnectEnabled = IsKickReconnectEnabled(m_Dialogs.GetCurrentKickDialog());
			
		//if (!reconnectEnabled)
		OnActionRefresh();
		
		// Join to invited server 
		Room invited = m_Lobby.GetInviteRoom();
		if (invited)
			JoinProcess_Init(invited);
		
		// Clear error msg 
		ClearErrorMessage();
		
		// Stop waiting for backend
		m_bIsWaitingForBackend = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fail wating for backend if takes too long 
	protected void ConnectionTimeout()
	{
		Messages_ShowMessage("NO_CONNECTION", true);
		
		if (m_Dialogs)
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.BACKEND_TIMEOUT);
			m_Dialogs.m_OnConfirm.Clear();
			m_Dialogs.m_OnConfirm.Insert(OnConnectionTimeoutDialogConfirm);
		}
		
		ClearConnectionTimeoutWaiting();
		
		m_bIsWaitingForBackend = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConnectionTimeoutDialogConfirm()
	{
		m_Dialogs.m_OnConfirm.Remove(OnConnectionTimeoutDialogConfirm);
		Messages_ShowMessage(TAG_MESSAGE_SEARCHING, true);
		m_bIsWaitingForBackend = false;
		
		GetGame().GetCallqueue().CallLater(ConnectionTimeout, BACKEND_CHECK_TIMEOUT);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearConnectionTimeoutWaiting()
	{
		GetGame().GetCallqueue().Remove(ConnectionTimeout);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Call this on receiving filtered rooms from search 
	//! Setting up server list - TODO@wernerjak - can be partialy moved in server list comp
	protected void OnRoomsFound(array<Room> rooms = null)
	{	
		// Allow rooms loading 
		m_bRoomsLoaded = true;
		
		// Load rooms 
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();
		
		if (rooms)
		{
			// Direct join rooms 
			lobby.Target(rooms);
		}
		else 
		{
			// Room from search
			lobby.Rooms(m_aRooms);
			m_CallbackAutoRefresh.m_OnSuccess.Insert(OnRoomAutoRefresh);
		}
		
		DisplayRooms(m_aRooms);
		
		// Move to top in list 
		if (m_ScrollableList)
		{	
			if (m_ScrollableList.IsListFocused() || m_bFirstRoomLoad)
			{
				m_ScrollableList.FocusFirstAvailableEntry();
			}
		}
		
		// Favorite filter is active? 
		/*if (m_ParamsFilter.GetFavoriteFilter())
			OnSearchFoundFavorite();
		
		// Recently played filter is active? 
		if (m_ParamsFilter.GetRecentlyPlayedFilter())
			OnSearchFoundFavorite();*/
			
		if (m_bFirstRoomLoad)
		{
			m_ScrollableList.SetIsListFocused(true);
			m_bFirstRoomLoad = false;
		}
		
		ClearConnectionTimeoutWaiting();
		m_ScrollableList.ShowScrollbar(true);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayRooms(array<Room> rooms = null)
	{		
		// Don't display if rooms are not loaded 
		if (!m_bRoomsLoaded)
			return;
		
		// Don't display if missing MP privilege
		if (!GetGame().GetPlatformService().GetPrivilege(UserPrivilege.MULTIPLAYER_GAMEPLAY))
		{
			return;
		}
		
		// Check list 
  		if (!m_ScrollableList || !m_ServerScenarioDetails)
		{
			PrintDebug("Missing important component references!", "DisplayRooms");
			return;	
		}
		
		// Check addons
		SCR_AddonManager addonMgr = SCR_AddonManager.GetInstance();
		
		if (!addonMgr)
		{
			PrintDebug("Could not find addon mngr to verify ugc privilege", "DisplayRooms");
			return;
		}
		
		// Modds filter when UGC not enabled 
		if (m_ParamsFilter.IsModdedFilterSelected() && !addonMgr.GetUgcPrivilege())
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_UGC");
			
			// Negotatiate UGC privilege 
			addonMgr.m_OnUgcPrivilegeResult.Insert(Platform_OnUgcPrivilegeResult);
			addonMgr.NegotiateUgcPrivilegeAsync();
			
			return;
		}
		
		
		int roomCount = m_Lobby.TotalRoomCount();
		
		// Display no rooms found 
		if (roomCount == 0)
		{
			// Display empty servers 
			m_ServerEntryFocused = null;
			m_EntryInteractible = null;

			// Setup list 
			m_ScrollableList.SetRooms(rooms, 0, true);
			m_ScrollableList.ShowEmptyRooms();
			
			if (m_ServerScenarioDetails)
				m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_EMPTY);
			
			// Show filters 
			m_FilterPanel.ShowFilterListBox(true);
			m_FilterPanel.EnableFilterButton(false);
			
			// Show message 
			Messages_ShowMessage("NO_FILTERED_SERVERS");
			RoomHandlingFavoriteEnable(false);
			
			return;
		}
		
		// Display rooms
		
		// Set rooms to fill server list 
		m_ScrollableList.SetRooms(m_aRooms, m_Lobby.TotalRoomCount(), true);
		
		// filters 
		m_FilterPanel.EnableFilterButton(true);
		
		// Hide message 
		Messages_Hide();
	}
	
	//
	// Lobby Rooms handling
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this once new room data are fetched 
	protected void OnRoomAutoRefresh(ServerBrowserCallback callback)
	{
		//DisplayRooms(m_aRooms);
		m_Lobby.SetRefreshRate(ROOM_REFRESH_RATE);
		DisplayRooms(m_aRooms);
		
		m_bWaitForRefresh = false;
		
		m_OnAutoRefresh.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for opening server detials dialog
	[MenuBindAttribute()]
	void OnActionServerDetails(SCR_NavigationButtonComponent navButton = null, string action = "")
	{		
		Room room = null;
		
		if (m_EntryInteractible)
			room = m_EntryInteractible.GetRoomInfo();
		
		if (!room)
			return;
		
		// Open connecting screen
		m_DetailsDialog = OpenServerDetailsDialog(m_EntryInteractible.GetRoomInfo());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for leaving menu 
	[MenuBindAttribute()]
	void OnActionBack()
	{
		// Remove specific filters 
		if (m_ParamsFilter)
		{
			m_ParamsFilter.SetScenarioId("");
			m_ParamsFilter.SetHostedScenarioModId("");
		}
		
		// Close 
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for refreshing server list
	[MenuBindAttribute()]
	void OnActionRefresh()
	{	
		// Try negotiate missing MP privilege
		if (!GetGame().GetPlatformService().GetPrivilege(UserPrivilege.MULTIPLAYER_GAMEPLAY))
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_MP", true);
			NegotiateMPPrivilegeAsync();
			return;
		}
		
		// Clear list 
		m_CallbackScroll.m_OnSuccess.Remove(OnScrollSuccess);
			
		//  Setup list loading
		if (m_ScrollableList)
		{
			m_ScrollableList.MoveToTop();
			m_ScrollableList.ShowEmptyRooms();
			m_ScrollableList.ShowScrollbar(false);
		}
		
		// Setup connection attempt timeout 
		GetGame().GetCallqueue().CallLater(ConnectionTimeout, BACKEND_CHECK_TIMEOUT);
		
		m_bRoomsLoaded = false;
		
		// Start loading 
		if (m_bFirstRoomLoad)
		{
			SearchRooms();
		}
		else
		{
			GetGame().GetCallqueue().CallLater(SearchRooms, ROOM_CONTENT_LOAD_DELAY, false);	
		}
		
		Messages_ShowMessage(TAG_MESSAGE_SEARCHING, m_bFirstRoomLoad);
		
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_SEACHING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for opening dialog with manual connect to ip
	[MenuBindAttribute()]
	void OnActionManualConnect()
	{
		MenuBase joinDialog = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.MultiplayerDialog, DialogPriority.CRITICAL);
		
		MultiplayerDialogUI multiplayerDialog = MultiplayerDialogUI.Cast(joinDialog);
		if (multiplayerDialog)
		{
			// Ip handling 	
			multiplayerDialog.m_OnConfirm.Clear();
			//multiplayerDialog.m_OnConfirm.Insert(OnDirectJoinConfirm);
			multiplayerDialog.m_OnConfirm.Insert(JoinActions_DirectJoin);
			
			// Cancel 
			multiplayerDialog.m_OnCancel.Clear();
			multiplayerDialog.m_OnCancel.Insert(OnDirectJoinCancel);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for switching to filter component
	[MenuBindAttribute()]
	void OnActionFilter()
	{
		// Set focus 
		if (m_iFocusedWidgetState != ESBWidgetFocus.FILTERING)
			SwitchFocus(null, ESBWidgetFocus.FILTERING);
		else 
			SwitchFocus(null, ESBWidgetFocus.SERVER_LIST);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bind action for favoriting server
	[MenuBindAttribute()]
	void OnActionFavorite()
	{
		if (!m_EntryInteractible)
			return;
		
		m_EntryInteractible.OnFavoriteClicked(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call searching for servers 
	protected void SearchRooms()
	{	
		// Clear auto refresh 
		m_CallbackAutoRefresh.m_OnSuccess.Clear();
		
		// Start loading and show loading feedback
		m_bRoomsLoaded = false;
		
		// Callback
		ref ServerBrowserCallback searchCallback = new ServerBrowserCallback;
		SetupSearchCallback(searchCallback);
		m_CallbackLastSearch = searchCallback;
		
		m_Lobby.SearchRooms(m_ParamsFilter, m_CallbackLastSearch);	
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Setup room search invoker actions and save callback in list 
	protected void SetupSearchCallback(ServerBrowserCallback searchCallback)
	{
		// Check callback 
		if (!searchCallback)
		{
			PrintDebug("Search callback is null!", "SetupSearchCallback");
			return;
		}
		
		// Invoker actions 
		searchCallback.m_OnSuccess.Insert(OnSearchRoomsSuccess);
		searchCallback.m_OnFail.Insert(OnSearchRoomsFail);
		searchCallback.m_OnTimeOut.Insert(OnSearchRoomsTimeOut);
		
		// Save callback 
		m_aSearchCallbacks.Insert(searchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsSuccess(ServerBrowserCallback callback)
	{
		PrintDebug(string.Format("Success - servers found: %1", m_Lobby.TotalRoomCount()), "OnSearchRoomsSuccess");
		
		// Cleanup callback data 
		RoomSearchCallbackCleanup(callback);

		// Display rooms
		OnRoomsFound();
		m_CallbackLastSearch = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		PrintDebug(string.Format("Room search fail - code: %1 | restCode: %2 | apiCode: %3", code, restCode, apiCode), "OnSearchRoomsFail");
		
		// Cleanup callback data 
		RoomSearchCallbackCleanup(callback);
	
		// Check if this is last request 
		if (callback != m_CallbackLastSearch)
			return;

		Messages_ShowMessage("BACKEND_SERVICE_FAIL", true);
		m_CallbackLastSearch = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsTimeOut()
	{
		ConnectionTimeout();
		PrintDebug("time out!", "OnSearchRoomsTimeOut");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clean up invoker actions and callback reference 
	protected void RoomSearchCallbackCleanup(ServerBrowserCallback callback)
	{
		callback.m_OnSuccess.Clear();
		callback.m_OnFail.Clear();
		
		m_aSearchCallbacks.RemoveItem(callback);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restore filtering parameters in UI
	void SetupParams(ClientLobbyApi lobby)
	{	
		FilteredServerParams params = FilteredServerParams.Cast(lobby.GetParameters());
		if (params == null)
		{
			// Default filter setup
			string strParams = lobby.GetStrParams();
			if (!strParams.IsEmpty())
			{
				m_ParamsFilter.ExpandFromRAW(strParams);
				lobby.ClearParams();
			}
		}
		else
		{
			// Restore previous filter setuo 
			m_ParamsFilter = params;
			SetupFilteringUI(m_ParamsFilter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply filter setup on each ui filtering widget - restoring filter UI states 
	protected void SetupFilteringUI(FilteredServerParams filterParams)
	{
		// Search
		if (m_SearchEditBox)
		{
			string searchStr = filterParams.GetSearchText();
			m_SearchEditBox.SetValue(searchStr);
		}
		
		// Sort
		if (m_SortBar)
		{
			string sortName = filterParams.GetSortOrder();
			int sortId = m_SortBar.FindElement(sortName);
			if (sortId != -1)
				m_SortBar.SetCurrentSortElement(sortId, ESortOrder.ASCENDING, useDefaultSortOrder: true);
		}
		
		// Tabview
		if (m_TabView)
		{
			if (m_ParamsFilter)
				m_TabView.ShowTab(m_ParamsFilter.GetSelectedTab());
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! General callbacks setup 
	protected void SetupCallbacks()
	{	
		// Loading 
		SCR_MenuLoadingComponent.m_OnMenuOpening.Insert(OnOpeningByLoadComponent);
		
		// Setup room data refresh
		m_Lobby.SetRefreshCallback(m_CallbackAutoRefresh);
		m_Lobby.SetRefreshRate(ROOM_REFRESH_RATE);
		
		// Server list
		m_ScrollableList.m_OnSetPage.Insert(CallOnServerListSetPage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayKick()
	{
		m_Dialogs.DisplayKickErrorDialog(m_sErrorMessage, m_sErrorMessageGroup, m_sErrorMessageDetail);

		// Find last 
		string lastId = m_Lobby.GetPreviousRoomId();
		bool hasLastServer = !lastId.IsEmpty();
		bool reconnectEnabled = IsKickReconnectEnabled(m_Dialogs.GetCurrentKickDialog());

		if (hasLastServer && reconnectEnabled)
		{
			m_Dialogs.m_OnConfirm.Insert(OnLastRoomReconnectConfirm);
			m_Dialogs.m_OnDialogClose.Remove(OnRejoinCancel);
			m_Dialogs.m_OnDialogClose.Insert(OnRejoinCancel);
			
			m_Dialogs.DisplayReconnectDialog(m_RejoinRoom);
		}
		
		// Clear messages and backend check 
		ClearErrorMessage();
		ClearConnectionTimeoutWaiting();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if reconnect can be displayed after this kick dialog 
	protected bool IsKickReconnectEnabled(SCR_ConfigurableDialogUi dialog)
	{
		// Check dialog 
		if (!dialog)
			return false;
		
		// Get kick comp
		SCR_KickDialogUiPreset kickPreset = SCR_KickDialogUiPreset.Cast(dialog.GetDialogPreset()); 
		if (!kickPreset)
			return false;
		
		// Result 
		return kickPreset.m_bCanBeReconnected;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when focusing on server 
	protected void OnServerEntryFocusEnter(SCR_ServerBrowserEntryComponent serverEntry)
	{
		if (!serverEntry || !m_ModsManager)
			return;
		
		m_ServerEntryFocused = serverEntry;

		// Compare previously focused entry 
		if (m_ServerEntryLastFocused != m_ServerEntryFocused)
		{
			// Start check new server 
			if (m_ModsManager)	
				m_ModsManager.DefaultSetup();
			
			m_ServerEntryLastFocused = m_ServerEntryFocused;
		}
		
		// Set interactible 
		if (m_EntryInteractible != m_ServerEntryFocused)
		{
			m_EntryInteractible = m_ServerEntryFocused;
		}
	
		// Focus room
		if (m_ServerEntryFocused)
		{
			m_iFocusedWidgetState = ESBWidgetFocus.SERVER_LIST;
		}
		
		// Load room data 
		Room room = m_ServerEntryFocused.GetRoomInfo();
		
		// Check restriction 
		bool canJoin = CanJoinRoom(room);
		
		m_OnEntryFocus.Invoke(true, canJoin, true);
		RoomHandlingButtonsEnabling(canJoin, true);
		RoomHandlingFavoriteEnable(true);
		
		m_LastFocusedRoom = room;
		ReceiveRoomContent(m_LastFocusedRoom, true, serverEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReceiveRoomContent(Room room,  bool receiveMods, SCR_ServerBrowserEntryComponent roomEntry)
	{
		// Check room
		if (!room && roomEntry && roomEntry.GetRoomInfo())
			room = roomEntry.GetRoomInfo();
		
		if (!room)
			return;
		
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayRoomData(room, receiveMods);
		
		// Check restriction 
		bool canJoin = CanJoinRoom(room);
		canJoin = true;
		
		m_OnEntryFocus.Invoke(true, canJoin, true);
		RoomHandlingButtonsEnabling(canJoin, true);
		
		// Call queue handling
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		
		// Clear previous call latter 
		callQueue.Remove(ReceiveRoomContent_Scenario);
		callQueue.Remove(ReceiveRoomContent_Mods);
		
		// Load scenario - vanilla instantly, modded with delay 
		if (room.HostScenarioMod())
		{	
			callQueue.CallLater(ReceiveRoomContent_Scenario, ROOM_CONTENT_LOAD_DELAY, false, room);
		}
		else
		{
			ReceiveRoomContent_Scenario(room);
		}
			
		// Check room mods count 
		array<Dependency> roomMod = {};
		room.AllItems(roomMod);
		
		// Is room modded 
		if (!roomMod.IsEmpty())
		{	
			// Show mod count immidiatelly 
			if (m_ServerScenarioDetails)
				m_ServerScenarioDetails.DisplayModsCount(roomMod.Count());
			
			// Receive mod data only if currently view is focused 
			if (room == m_LastFocusedRoom)
			{
				// Receive cached mod data if rooms last checked 
				if (room == m_LastLoadedRoom)
				{
					ReceiveRoomContent_Mods(room);
					return;
				}
				
				// Load mods after short delay - to prevent spamming mods receive requiest with fast server selecting
				callQueue.CallLater(ReceiveRoomContent_Mods, ROOM_CONTENT_LOAD_DELAY, false, room);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Separated receive scenario data for server 
	//! Call this later if scenario is from mod to prevent too many request 
	protected void ReceiveRoomContent_Scenario(Room room)
	{
		if (!room)
			return;
		
		if (m_LastFocusedRoom != room)
		{	
			// Modded scenario - prevent display, wait for focus
			// TODO@wernerjak - improve modded scenario check 
			if (room.HostScenarioMod())  
				return;
		}
		
		// Load scenario 
		m_ModsManager.m_OnGettingScenario.Insert(OnLoadingScenario);
		m_ModsManager.ReceiveRoomScenario(room);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Separeted receive mods data for server 
	//! Call this later to prevent too many request 
	protected void ReceiveRoomContent_Mods(Room room)
	{
		if (!m_ModsManager || !room || m_LastFocusedRoom != room)
			return;
		
		// Check cached dependencies 
		//if (m_ModsManager.AllItemsLoaded)
		
		m_ModsManager.m_OnGettingAllDependecies.Insert(OnLoadingDependencyList);
		m_ModsManager.ReceiveRoomMods(room);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when leaving focus 
	protected void OnServerEntryFocusLeave(SCR_ServerBrowserEntryComponent serverEntry)
	{
		// Invoke focus leave 
		m_OnEntryFocus.Invoke(false, false, false);
		
		RoomHandlingButtonsEnabling(false, false);
		RoomHandlingFavoriteEnable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Join to the room on double click on server entry
	protected void OnServerEntryDoubleClick(SCR_ServerBrowserEntryComponent entry)
	{
		// Join 
		JoinActions_Join();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryMouseEnter(SCR_ServerBrowserEntryComponent entry)
	{
		m_EntryInteractible = entry;
		
		if (entry && entry != m_ServerEntryFocused)
		{
			if (!entry.GetRoomInfo())
				return;
			
			ReceiveRoomContent(entry.GetRoomInfo(), false, entry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryMouseLeave(SCR_ServerBrowserEntryComponent entry)
	{
		m_EntryInteractible = m_ServerEntryFocused;
		
		if (entry != m_ServerEntryFocused && m_LastFocusedRoom)
		{
			ReceiveRoomContent(m_LastFocusedRoom, true, entry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getting reference for all server widget elements 
	protected void SetupHandlers()
	{
		// Top filtering widgets --------------------
		
		// Tab view 
		m_TabView = SCR_TabViewComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_TAB_VIEW, SCR_TabViewComponent));
		if (m_TabView)
			m_TabView.m_OnChanged.Insert(OnTabViewSwitch);
		
		// Filter panel 
		m_FilterPanel = SCR_FilterPanelComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_FILTER, SCR_FilterPanelComponent));
		if (m_FilterPanel)
		{	
			// Invoker 
			m_FilterPanel.m_OnFilterPanelToggled.Insert(OnFilterPanelToggle);
			m_FilterPanel.m_OnFilterChanged.Insert(OnChangeFilter);
			
			// Attempt load previous filter setup 
			m_FilterPanel.TryLoad();
			m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
		}
		
		// Sorting header 
		m_SortBar = SCR_SortHeaderComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_SERVER_HEADER, SCR_SortHeaderComponent));
			
		if (m_SortBar)
		{
			m_SortBar.m_OnChanged.Insert(OnChangeSort);
			
			// Initial sort
			// todo move default sorting values out of here, it can be set in layout file now
			m_SortBar.SetCurrentSortElement(1, ESortOrder.ASCENDING);
		}
		 
		// Search edit box
		m_SearchEditBox = SCR_EditBoxSearchComponent.Cast(
			m_Widgets.FindHandlerReference(m_Widgets.m_wSearchEditBox, m_Widgets.WIDGET_SEARCH, SCR_EditBoxSearchComponent)
		);
		
		if (m_SearchEditBox)
			m_SearchEditBox.m_OnConfirm.Insert(OnSearchEditBoxConfirm);
		
		// Server list and scroll --------------------
		m_ScrollableList = SCR_PooledServerListComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SCROLLABLE_LIST, SCR_PooledServerListComponent
		));
		
		if (m_ScrollableList)
			m_Lobby.SetViewSize(m_ScrollableList.GetPageEntriesCount() * 2);
	
		// Add callbacks 
		array<SCR_ServerBrowserEntryComponent> entries = m_ScrollableList.GetRoomEntries(); 
			
		foreach (SCR_ServerBrowserEntryComponent entry : entries)
		{
			// Set invoker actions 
			entry.Event_OnFocusEnter.Insert(OnServerEntryFocusEnter);
			entry.Event_OnFocusLeave.Insert(OnServerEntryFocusLeave);
			entry.m_OnFavorite.Insert(OnRoomEntrySetFavorite);
			entry.m_OnDoubleClick.Insert(OnServerEntryDoubleClick);
			entry.m_OnMouseEnter.Insert(OnServerEntryMouseEnter);
			entry.m_OnMouseLeave.Insert(OnServerEntryMouseLeave);
		}
		
		// Detail screen
		m_ServerScenarioDetails = SCR_ServerScenarioDetailsPanelComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SERVER_SCENARIO_DETAILS_PANEL, SCR_ServerScenarioDetailsPanelComponent
		));
		
		// Bacis setup and hide 
		if (m_ServerScenarioDetails)
		{
			m_ServerScenarioDetails.SetModsManager(m_ModsManager);
		}
		
		// Messages 
		m_SimpleMessageWrap = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_WRAP, SCR_SimpleMessageComponent));

		m_SimpleMessageList = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_LIST, SCR_SimpleMessageComponent));
		
		// Bottom navigation bar --------------------
		
		// Navigation buttons 
		m_BtnJoin = SCR_NavigationButtonComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_BUTTON_JOIN, SCR_NavigationButtonComponent
		));
		
		/*if (m_BtnJoin) 
			m_BtnJoin.m_OnActivated.Insert(QuickJoinRoom);*/
		if (m_BtnJoin)
			m_BtnJoin.m_OnActivated.Insert(JoinActions_Join);
		
		m_BtnDetails = SCR_NavigationButtonComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_BUTTON_DETAILS, SCR_NavigationButtonComponent
		));
		
		if (m_BtnDetails)
			m_BtnDetails.m_OnActivated.Insert(OnActionServerDetails);
		
		m_BtnFavorite =  SCR_NavigationButtonComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_BUTTON_FAVORITE, SCR_NavigationButtonComponent
		));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch focus to target widget to handling
	protected void SwitchFocus(Widget wFocused, ESBWidgetFocus focus)
	{
		// Remove if focus is already applied 
		if (m_iFocusedWidgetState == focus)
			return;
	
		// Quick focus on widget 	
		if (wFocused)
		{
			GetGame().GetWorkspace().SetFocusedWidget(wFocused);
			return;
		}
		
		// Filter 
		if (m_FilterPanel)
			m_FilterPanel.ShowFilterListBox(false);
		
		// Set focus stat 
		m_iFocusedWidgetState = focus;
		
		// Setup focus
		switch (focus)
		{
			// Server list - base
			case ESBWidgetFocus.SERVER_LIST:
			{
				if (m_wFirstServerEntry)
					wFocused = m_wFirstServerEntry;
				
				break;
			}
			
			// Filtering 
			case ESBWidgetFocus.FILTERING:
			{
				if (m_FilterPanel)
				{
					m_FilterPanel.ShowFilterListBox(true);
					
					Widget button = m_FilterPanel.m_Widgets.m_FilterButton;
					if (button)
						GetGame().GetWorkspace().SetFocusedWidget(button);
				}
				break;
			}
				
			// Sorting
			case ESBWidgetFocus.SORTING:
			{
				if (m_SortBar)
				{
					//wFocused = m_SortBar.GetLastFocusedButton();m_wRoot
					wFocused = m_Widgets.m_wSortSessionName;
				}
				break;
			}
				
			// Search box
			case ESBWidgetFocus.SEARCH:
			{
				if (m_Widgets.m_wSearchEditBox)
					wFocused = m_Widgets.m_wSearchEditBox;
				break;
			}
		}
		
		// Switch focus 
		if (wFocused)
		{
			GetGame().GetWorkspace().SetFocusedWidget(wFocused);
			m_iFocusedWidgetState = focus;
		}
		else
		{
			// Go to server list first entry by default
			if (m_wFirstServerEntry)
			{
				GetGame().GetWorkspace().SetFocusedWidget(m_wFirstServerEntry);
				m_iFocusedWidgetState = ESBWidgetFocus.SERVER_LIST;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set focus to last filtering element
	protected void OnChangeSort(SCR_SortHeaderComponent sortHeader)
	{
		if (!m_ParamsFilter)
			return;
	
		// Reload data
		bool sortAscending = sortHeader.GetSortOrderAscending();
		string sortElementName = sortHeader.GetSortElementName();
		m_ParamsFilter.SetSorting(sortElementName, sortAscending);

		if (!m_bFirstRoomLoad)
			OnActionRefresh();

		SwitchFocus(null, ESBWidgetFocus.SORTING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set tab filters on switching tab view 
	void OnTabViewSwitch(SCR_TabViewComponent tabView, Widget w, int id)
	{	
		// Default setup 
		m_ParamsFilter.SetFavoriteFilter(false);
		m_ParamsFilter.SetRecentlyPlayedFilter(false); 
		
		switch (id)
		{
			// All servers 
			case 0:
				break;
			
			// Favorite servers 
			case 1:
			{
				m_ParamsFilter.SetFavoriteFilter(true);
				break;
			}
			
			// Recently played 
			case 2:
			{
				m_ParamsFilter.SetRecentlyPlayedFilter(true); 
				break;
			}
		}
		
		// Set tab filter json
		if (m_ParamsFilter)
			m_ParamsFilter.SetSelectedTab(id);
		
		OnActionRefresh();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterPanelToggle(bool show)
	{
		// Focus back to server list 
		if (show || m_Lobby.TotalRoomCount() == 0)
			return; 
		
		// Focus back to last entry 
		if (m_ServerEntryFocused)
		{
			RefocusEntry();
			return;
		}
		
		// Focus first  
		if (!m_ScrollableList)
			return;
		
		Widget first = m_ScrollableList.FirstAvailableEntry();
		if (first)
			GetGame().GetWorkspace().SetFocusedWidget(first);
	}
	
	protected bool m_bWasFilterChanged = false;
	
	//------------------------------------------------------------------------------------------------
	//! Call this when any of filter in filter panel is changed
	//! Get last updated filter and set it up for next search
	protected void OnChangeFilter()
	{
		// Set filter and refresh		
		m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
		OnActionRefresh(); 
		
		// Save filter 
		m_FilterPanel.Save();
		
		m_bWasFilterChanged = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CallOnServerListSetPage(int page)
	{
		GetGame().GetCallqueue().Remove(OnServerListSetPage);
		GetGame().GetCallqueue().CallLater(OnServerListSetPage, ROOM_CONTENT_LOAD_DELAY, false, page);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this actions when server list page is changed
	protected void OnServerListSetPage(int page)
	{
		// Check list 
		if (!m_ScrollableList)
			return;
		
		// Get entries count
		int entriesC = m_ScrollableList.GetPageEntriesCount();
		int pos = entriesC * page;

		// Setup scroll callback 
		m_CallbackScroll.m_OnSuccess.Insert(OnScrollSuccess);

		if (!m_bFirstRoomLoad)
		{
			array<Room> rooms = {};
			m_Lobby.Rooms(rooms);
			m_Lobby.Scroll(pos, m_CallbackScroll);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnScrollSuccess(ServerBrowserCallback callback)
	{
		// Check if loaded room should display details
		bool loadedRoomFocused = false;
		if (m_ServerEntryFocused && !m_ScrollableList.IsRoomLoaded(m_ServerEntryFocused))
			loadedRoomFocused = true;
		
		// Update rooms
		m_Lobby.Rooms(m_aRooms);
		m_ScrollableList.UpdateLoadedPage();
		
		DisplayRooms(m_aRooms);
		 
		// Clear callback 
		m_CallbackScroll.m_OnSuccess.Remove(OnScrollSuccess);
		
		
		// Focus on new room 
		if (loadedRoomFocused)
			ReceiveRoomContent(m_LastFocusedRoom, true, m_ServerEntryFocused);
	}
	
	protected ref array<ref SCR_FilterEntry> m_aFiltersToSelect = new array<ref SCR_FilterEntry>;
	
	protected bool m_bIsCheckingSpecificfilter = false;
	
	//------------------------------------------------------------------------------------------------
	//! Call this when search is sucessful with favorite filter active 
	protected void OnSearchFoundFavorite()
	{		
		if (!m_bIsCheckingSpecificfilter)
		{
			if (m_Lobby.TotalRoomCount() == 0)
			{
				m_bIsCheckingSpecificfilter = true;
				
				m_ParamsFilter.DefaulFilterFavorite();
				OnActionRefresh();
				
				SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
				m_ParamsFilter.SetFilters(filterSet);
			}
		}
		else
		{
			m_OnAutoRefresh.Clear();
			m_CallbackAutoRefresh.m_OnSuccess.Clear();
			
			// Show favorite missing specific message
			if (m_Lobby.TotalRoomCount() == 0)
				Messages_ShowMessage("NO_FAVORITES");
			else 
			{
				Messages_ShowMessage("NO_FILTERED_SERVERS");
			}
	
			m_bIsCheckingSpecificfilter = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when search is sucessful with recently played filter active 
	protected void OnSearchFoundRecentlyPlayed()
	{
		m_ParamsFilter.DefaulFilterRecentlyPlayed();
		
		// Is double checked?
		if (!m_bIsCheckingSpecificfilter)
		{
			m_bIsCheckingSpecificfilter = true;
			OnActionRefresh();
		}
		else
		{
			// Show favorite missing specific message
			
			m_bIsCheckingSpecificfilter = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find filter by it's internal name define in ServerBrowserFilterSet.conf
	//! Set filter enabled 
	void ActivateFilter(string filterName, bool enabled, bool processFilters = true)
	{
		// Setup filter 
		SCR_FilterEntry filter = new SCR_FilterEntry;
		filter.m_sInternalName = filterName;
		filter.SetSelected(true);
		
		// Register filter 
		m_aFiltersToSelect.Insert(filter);
		
		// Activate filter if possible 
		if (processFilters)
			ActivateFiltersFromList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateFiltersFromList()
	{
		// Filters array check 
		if (!m_aFiltersToSelect || m_aFiltersToSelect.IsEmpty())
			return;
		
		// Filter panel check 
		if (!m_FilterPanel || !m_ParamsFilter)
			return;
		
		// Get filter set 
		ref SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
		if (!filterSet)
			return;
		
		// Check and enable each filter 
		foreach (SCR_FilterEntry filter : m_aFiltersToSelect)
		{
			if (!filter)
				continue;
			
			// Find filter to select
			SCR_FilterEntry targetFilter = filterSet.FindFilter(filter.m_sInternalName);
			
			// Set select if is found 
			if (targetFilter && targetFilter.GetSelected() != filter.GetSelected())
			{
				targetFilter.SetSelected(filter.GetSelected());
				m_FilterPanel.SelectFilter(targetFilter, filter.GetSelected());
			}
		}
		
		// Clear filters 
		m_aFiltersToSelect.Clear();
		
		// Filter 
		m_ParamsFilter.SetFilters(filterSet);
		OnActionRefresh(); 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fetch new servers on confirming search string
	protected void OnSearchEditBoxConfirm(SCR_EditBoxComponent editBox, string sInput)
	{
		m_ParamsFilter.SetSearch(sInput);
		OnActionRefresh();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Favoriting server 
	protected void OnRoomEntrySetFavorite(notnull SCR_ServerBrowserEntryComponent serverEntry, bool favorite)
	{
		Room roomClicked = serverEntry.GetRoomInfo();
		if (!roomClicked)
			return;
		
		int roomId = m_aRooms.Find(roomClicked);	
		
		if (roomId != -1)
			m_aRooms[roomId].SetFavorite(favorite, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open server details and setup it
	ServerDetailsMenuUI OpenServerDetailsDialog(Room room)
	{
		//ServerBrowserDetailsUI connectMenu = null;
		ServerDetailsMenuUI connectMenu = null;
		
		// Open connecting screen
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserDetailsUI, DialogPriority.CRITICAL);

		if(menu)
		{
			// Setup connection menu
			connectMenu = ServerDetailsMenuUI.Cast(menu);
			
			if(connectMenu)
			{
				connectMenu.SetServerDetail(this, m_ModsManager, room);
				connectMenu.m_OnOpen.Insert(OnDetailsMenuOpen);
			}
		}
		
		return connectMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when connect menu finish its opening 
	protected void OnDetailsMenuOpen(ServerDetailsMenuUI detailsUI, bool opened, Room room)
	{
		m_bConnectMenuOpened = opened;
		
		// Opening menu
		if (m_bConnectMenuOpened)
		{
			if (m_bIsScenarioLoaded)
				m_OnScenarioLoad.Invoke(m_RoomScenario);
			
			// Prevent menu actions 
			if (m_TabView)			
				m_TabView.SetListenToActions(false);
		}
		
		// closing menu
		if (!opened)
		{
			if (room)
			{
				// Find last entry by last opened server connect
				SCR_ServerBrowserEntryComponent lastEntry = null;
				foreach (SCR_ServerBrowserEntryComponent entry : m_aRoomEntryList)
				{
					if (entry.GetRoomInfo() == room)
					{
						lastEntry = entry;
					}
				}
				
				// Focus on last selected entry 
				if (lastEntry)
					m_ServerEntryFocused = lastEntry;
				
				// Server browser is reopened 
				m_bServerBrowserReopened = true;
			}
			
			// Allow tabview use
			if (m_TabView)
				m_TabView.SetListenToActions(true);
			
			// Clear invokers 
			if (detailsUI.m_OnOpen)
				detailsUI.m_OnOpen.Remove(OnDetailsMenuOpen);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mods list is received 
	protected void OnLoadingDependencyList()
	{
		// Remove data receiving actions 
		m_ModsManager.m_OnGettingAllDependecies.Remove(OnLoadingDependencyList);
		
		// Setup variables 
		m_bServerItemsLoaded = true;
		
		array<ref SCR_WorkshopItem> updated = m_ModsManager.GetRoomItemsUpdated();
		array<ref SCR_WorkshopItem> outdated = m_ModsManager.GetRoomItemsToUpdate();
		
		// setup of server detail 
		bool modsUpdated = outdated.IsEmpty();
		
		// Invoke getting dependencies 
		m_OnDependeciesLoad.Invoke();
		
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayMods();
		
		// Save room with last loaded info 
		if (m_ServerEntryFocused)
			m_LastLoadedRoom = m_ServerEntryFocused.GetRoomInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mods loading got error 
	//! Show mods loading fail message 
	protected void OdModListFail(bool success)
	{
		m_OnDependeciesLoad.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setting server details 
	//! Call this when room receive scenrio data 
	protected void OnLoadingScenario(Dependency scenario)
	{
		// Remove action
		m_ModsManager.m_OnGettingScenario.Remove(OnLoadingScenario);
		
		// Set server details
		Room roomInfo = null;
		
		if (m_EntryInteractible)
			roomInfo = m_EntryInteractible.GetRoomInfo();
		
		if (!roomInfo)
			return;
		
		// Get scenario item 
		MissionWorkshopItem scenarioItem = roomInfo.HostScenario();
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance(); 
		
		// Hide scneario img if scenario if modded client can't see UGC
		Dependency scenarioMod = roomInfo.HostScenarioMod();
		bool hideScenario = scenarioMod && !mgr.GetUgcPrivilege();
		
		// Display room and scenario data
		if (m_ServerScenarioDetails)
		{
			// Hide scenario image
			m_ServerScenarioDetails.SetHideScenarioImg(hideScenario);
			
			if (scenarioItem) // Vanilla 
				m_ServerScenarioDetails.SetScenario(scenarioItem);
			else // Modded  
				m_ServerScenarioDetails.SetScenario(scenarioItem);
		}
		
		m_RoomScenario = scenarioItem;
		m_bIsScenarioLoaded = true;
		
		m_OnScenarioLoad.Invoke(m_RoomScenario);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Focus on last selected server entry -TODO@wernerjak - move this to server list component 
	//! Call this when getting back to server browser - from server details
	protected void RefocusEntry()
	{
		if (m_ServerEntryFocused)
			GetGame().GetWorkspace().SetFocusedWidget(m_ServerEntryFocused.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set enable for room handling buttons in various states 
	protected void RoomHandlingButtonsEnabling(bool joinEnabled, bool detailsEnabled)
	{
		if (!m_BtnJoin || !m_BtnDetails)
			return;
		
		m_BtnJoin.SetEnabled(joinEnabled);
		m_BtnDetails.SetEnabled(detailsEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RoomHandlingFavoriteEnable(bool enable)
	{
		if (m_BtnFavorite)
			m_BtnFavorite.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! Show message feedback by selected message component 
	//! showWrap = true will hide server list and display message on full layout
	protected void Messages_ShowMessage(string messageTag, bool showWrap = false)
	{
		Widget wrapLayout = m_Widgets.m_wPanelEmpty;
		
		// Check message component references 
		if (!wrapLayout || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", wrapLayout, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_ShowMessage");
			return; 
		}
		
		// Show menu content 
		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(!showWrap);
		
		// Display messages 
		wrapLayout.SetVisible(showWrap);
		m_SimpleMessageWrap.GetRootWidget().SetVisible(showWrap);
		
		m_SimpleMessageList.GetRootWidget().SetVisible(!showWrap);
		
		// Wrapper 
		if (showWrap)
		{	
			m_SimpleMessageWrap.SetContentFromPreset(messageTag);
			//return;
		}
		
		// List
		m_SimpleMessageWrap.SetContentFromPreset(messageTag);
		m_SimpleMessageList.SetContentFromPreset(messageTag);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hide both message widgets
	protected void Messages_Hide()
	{
		Widget wrapLayout = m_Widgets.m_wPanelEmpty;
		
		// Check message component references 
		if (!wrapLayout || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", wrapLayout, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_Hide");
			return; 
		}
		
		// Hide 
		m_SimpleMessageWrap.GetRootWidget().SetVisible(false);
		m_SimpleMessageList.GetRootWidget().SetVisible(false);
		
		wrapLayout.SetVisible(false);
		
		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! On join callback fail show join error dialog 
	protected void OnJoinFail(int code, int restCode, int apiCode)
	{
		if (m_Dialogs)
			m_Dialogs.DisplayJoinFail(apiCode);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if is possible to attempt to join room
	//! Returns true if room version and platform match to client
	protected bool CanJoinRoom(Room room)
	{
		if (!room)
		{
			PrintDebug("No room found for can join room check", "CanJoinRoom");
			return false;
		}
		
		// Room to client version check 
		bool versionMatch = ClientRoomVersionMatch(room);
		
		//TODO@wernerjak - check platform restriction 
		
		return versionMatch;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ClientRoomVersionMatch(Room room)
	{
		if (!room)
			return false;
		
		string clientV = GetGame().GetBuildVersion();
		string roomV = room.GameVersion();
		
		
		return (clientV == roomV);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this on rejoin dialog confirm 
	protected void OnRejoinConfirm(SCR_NavigationButtonComponent btn)
	{
		if (m_RejoinRoom)
			JoinProcess_Init(m_RejoinRoom);
		
		if (m_Dialogs.GetCurrentKickDialog())
		{
			m_Dialogs.m_OnConfirm.Remove(OnRejoinConfirm);
			m_Dialogs.m_OnCancel.Remove(OnRejoinCancel);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!  Call this on rejoin dialog cancel 
	protected void OnRejoinCancel()
	{
		OnActionRefresh();
		
		if (m_Dialogs.GetCurrentKickDialog())
		{
			m_Dialogs.m_OnConfirm.Remove(OnLastRoomReconnectConfirm);
			m_Dialogs.m_OnDialogClose.Remove(OnRejoinCancel);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this to progress to join once all mods are downloaded in join dialogs 
	//! This should be pnly called if dialog is still open 
	protected void OnDialogsHandlerDownloadComplete(Room room)
	{
		//JoinRoom(room);
		JoinProcess_CheckRoomPasswordProtected(room);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this on closing on of join dialog handler dialogs to stop interacting with dialog 
	protected void OnDialogsHandlerClose()
	{
		/*if (m_Dialogs)
			m_Dialogs.m_OnDownloadComplete.Clear();*/
		
		// Clean invite 
		if (m_Lobby.GetInviteRoom())
		{
			m_Lobby.ClearInviteRoom();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display again old search if servers were overidden by direct join search 
	protected void OnDirectJoinCancel()
	{
		// TODO@wernerjak - restore old search
	}
	
	// API
	//------------------------------------------------------------------------------------------------
	Room GetJoiningRoom() { return m_JoiningRoom; }
	
	//------------------------------------------------------------------------------------------------
	void SetJoiningRoom(Room room) { m_JoiningRoom = room; }
	
	//------------------------------------------------------------------------------------------------
	void FilterScenarioId(string scenarioId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetScenarioId(scenarioId);
	}
	
	//------------------------------------------------------------------------------------------------
	void FilterHostedScenarioModId(string scenarioModId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetHostedScenarioModId(scenarioModId);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Set to find server by scenario id and scenario source mod id 
	static void OpenWithScenarioFilter(string scenarioId, string scenarioModId = string.Empty)
	{
		ServerBrowserMenuUI sb = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (sb)
		{		
			sb.FilterScenarioId(scenarioId);
			
			// Modded 
			if (scenarioModId != string.Empty)
				sb.FilterHostedScenarioModId(scenarioModId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set to find server by mission workshop item
	static void OpenWithScenarioFilter(MissionWorkshopItem mission)
	{
		ServerBrowserMenuUI sb = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (!sb)
			return;
		
		// Scenario id
		string scenarioId = mission.Id();
		sb.FilterScenarioId(scenarioId);
			
		// Modded 
		string modId = "";
		WorkshopItem owner = mission.GetOwner();
		if (owner)
			modId = owner.Id();
		
		if (modId != string.Empty)
			sb.FilterHostedScenarioModId(modId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when server browser is open by loading component
	//! Start waiting for backend
	protected void OnOpeningByLoadComponent(int menuPreset)
	{
		// Check menu opening 
		if (menuPreset == ChimeraMenuPreset.ServerBrowserMenu)
			return;
	
		// Set wait for backemd
		m_bIsWaitingForBackend = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set message for error dialog
	//! Error is display if message has text, message is cleared after displaying dialog  
	static void SetErrorMessage(string msg, string group, string details = "") 
	{
		m_sErrorMessage = msg;
		m_sErrorMessageGroup = group;
		m_sErrorMessageDetail = details;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearErrorMessage()
	{
		m_sErrorMessage = "";
		m_sErrorMessageGroup = "";
		m_sErrorMessageDetail = "";
	}
	
	//------------------------------------------------------------------------------------------------
	void ServerBrowserMenuUI() {}
	
	
	void ~ServerBrowserMenuUI() {}
	
	//------------------------------------------------------------------------------------------------
	// Room joining process
	//------------------------------------------------------------------------------------------------

	
	//------------------------------------------------------------------------------------------------
	//! Action for joining to selected server 
	protected void JoinActions_Join()
	{
		// Prevent join if no entry selected 
		if (!m_EntryInteractible)
		{
			PrintDebug("Quick join is not possible because there is no selected server", "JoinActions_Join");
			return;
		}
		
		// Loaded mods if interactible is hovered 
		if (m_EntryInteractible != m_ServerEntryFocused)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_EntryInteractible.GetRootWidget());
			ReceiveRoomContent(m_EntryInteractible.GetRoomInfo(), false, m_EntryInteractible);
		}
		
		// Join
		Room roomToJoin = m_EntryInteractible.GetRoomInfo();
		if (roomToJoin)
			JoinProcess_Init(roomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	// Action for finding server on direct join 
	protected void JoinActions_DirectJoin(string params, EDirectJoinFormats format, bool publicNetwork)
	{
		JoinProcess_FindRoom(params, format, publicNetwork)
	}
	
	//------------------------------------------------------------------------------------------------
	//! Confirm action to find new server 
	protected void OnLastRoomReconnectConfirm()
	{
		string lastId = m_Lobby.GetPreviousRoomId();
		JoinProcess_FindRoomById(lastId, m_CallbackSearchPreviousRoom);
		
		m_Dialogs.m_OnConfirm.Remove(OnLastRoomReconnectConfirm);
	}
	
	protected Room m_RoomToJoin;
	protected EDirectJoinFormats m_eLastJoinFormat = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room 
	void JoinProcess_FindRoom(string params, EDirectJoinFormats format, bool publicNetwork)
	{		
		m_DirectJoinParams = new FilteredServerParams;
		m_eLastJoinFormat = format;
		
		// Format 
		switch (format)
		{
			// IP:PORT
			case EDirectJoinFormats.IP_PORT:
			{
				m_DirectJoinParams.SetHostAddress(params);
				break;
			}
			
			// Direct join code
			case EDirectJoinFormats.JOIN_CODE:
			{
				m_DirectJoinParams.SetJoinCode(params);
				break;
			}
			
			// Invalid format 
			case EDirectJoinFormats.INVALID:
			{
				PrintDebug("Invalid format of direct join", "JoinProcess_FindRoom");
				return;
			}
		}
		
		// Set room search
		m_Lobby.SearchTarget(m_DirectJoinParams, m_CallbackSearchTarget);
		
		// Setup join dialog 
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);
		
		// Set invokers
		m_CallbackSearchTarget.m_OnSuccess.Insert(JoinProcess_OnFindRoomSuccess);
		m_CallbackSearchTarget.m_OnFail.Insert(JoinProcess_OnFindRoomFail);
		m_CallbackSearchTarget.m_OnTimeOut.Insert(JoinProcess_OnFindRoomTimeout);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when room is found 
	protected void JoinProcess_OnFindRoomSuccess(ServerBrowserCallback callback)
	{
		m_aDirectFoundRooms = m_CallbackSearchTarget.GetFoundRooms();
		if (m_aDirectFoundRooms.IsEmpty())
		{
			PrintDebug("No room found through direct join!", "JoinProcess_OnFindRoomSuccess");
			return;
		}
		
		// Check if room was found 
		if (!m_aDirectFoundRooms || m_aDirectFoundRooms.Count() < 1)
		{
			if (m_Dialogs)
				m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);
			
			return;
		}
		
		if (m_aDirectFoundRooms.Count() == 1)
		{
			// Check content of specific server 
			m_ModsManager.DefaultSetup();
			//VerifyRoomContentDialog(rooms[0]);
			//VerifyRoomPasswordDialog(rooms[0]);
			
			m_RoomToJoin = m_aDirectFoundRooms[0];
			JoinProcess_Init(m_RoomToJoin);
		}
		else
		{
			// Show multiple rooms in list - fallback logic, shouldn't happend in relased game 
			OnRoomsFound(m_aDirectFoundRooms);
		}
		
		// Clear direct join callback
		JoinProcess_CleanFindCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if room wasn't found because of error or time out
	protected void JoinProcess_OnFindRoomFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);
		
		JoinProcess_CleanFindCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room has timeout
	protected void JoinProcess_OnFindRoomTimeout(ServerBrowserCallback callback)
	{
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);
		
		JoinProcess_CleanFindCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CleanFindCallback()
	{
		m_CallbackSearchTarget.m_OnSuccess.Remove(JoinProcess_OnFindRoomSuccess);
		m_CallbackSearchTarget.m_OnFail.Remove(JoinProcess_OnFindRoomFail);
		m_CallbackSearchTarget.m_OnTimeOut.Remove(JoinProcess_OnFindRoomTimeout);
		
	 	//m_Dialogs.m_OnDialogClose.Remove(OnRejoinCancel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Specific call for id search 
	protected void JoinProcess_FindRoomById(string id, ServerBrowserCallback callback)
	{
		// Setup ID
		m_SearchIds.ClearIds();
		m_SearchIds.RegisterId(id);
		
		// Search 
		m_Lobby.GetRoomsByIds(m_SearchIds, callback);
		callback.event_OnResponse.Insert(JoinProcess_OnFindRoomByIdResponse);
		
		// Setup join dialog 
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reaction for rooms found by id response 
	protected void JoinProcess_OnFindRoomByIdResponse(ServerBrowserCallback callback)
	{
		switch (callback.GetResultType())
		{
			// Success 
			case EServerBrowserRequestResult.SUCCESS:
			{
				m_Lobby.Rooms(m_aDirectFoundRooms);
				
				// No rooms received 
				if (m_aDirectFoundRooms.IsEmpty())
				{
					JoinProcess_OnFindRoomFail(null, -1, -1, -1);
					break;
				}
				
				// Get first room 
				m_RoomToJoin = m_aDirectFoundRooms[0];
				m_RejoinRoom = m_RoomToJoin;
				JoinProcess_Init(m_RoomToJoin);
				break;
			}
			
			// Error 
			case EServerBrowserRequestResult.ERROR:
			{
				JoinProcess_OnFindRoomFail(null, -1, -1, -1);
				break;
			}
			
			// Timeout 
			case EServerBrowserRequestResult.TIMEOUT:
			{
				JoinProcess_OnFindRoomFail(null, -1, -1, -1);
				break;
			}
		}
		
		// Clear
		callback.event_OnResponse.Remove(JoinProcess_OnFindRoomByIdResponse);
	}
	
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room 
	void JoinProcess_Init(Room roomToJoin)
	{	
		// Set immidiate refresh to receive player count 
		//m_Lobby.SetRefreshRate(1);
		m_bWaitForRefresh = true;
		
		// Setup join dialog 
		SetupJoinDialogs();
		
		// Setup room and next step
		m_RoomToJoin = roomToJoin;
		JoinProcess_CheckVersion(m_RoomToJoin);
		
		if (m_Lobby.GetInviteRoom())
		{
			PrintDebug("Server browser is joining to invited server!", "JoinProcess_Init");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if client and room versions match
	//! Progress only if versions are matching, otherwise show error and stop progress 
	protected void JoinProcess_CheckVersion(Room roomToJoin)
	{
		// Chekc match
		bool versionsMatch = ClientRoomVersionMatch(roomToJoin);
		
		// Stop join process with error dialog with wrong version
		if (!versionsMatch)
		{
			m_Dialogs.SetJoinRoom(roomToJoin);
			m_Dialogs.DisplayDialog(EJoinDialogState.VERSION_MISMATCH);
			return;
		}
		
		// Check room password protection - TODO@wernerjak - add password validation before content check
		//JoinProcess_CheckRoomPasswordProtected(roomToJoin);
		JoinProcess_LoadModContent(roomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if room requires password to join
	//! Open password dialog room is password protected and password is not skipped, otherwiser next step
	protected void JoinProcess_CheckRoomPasswordProtected(Room roomToJoin)
	{
		// Skip password if using direct join code or is invited
		bool skipPassword = false;
		
		if (m_Lobby.GetInviteRoom() == m_RoomToJoin || m_RejoinRoom)
			skipPassword = true;
		
		// Next step if no password protection
		if (!m_RoomToJoin.PasswordProtected() || skipPassword)
		{
			JoinProcess_Join(roomToJoin);
			return;
		}
		
		// Open password dialog 
		JoinProcess_PasswordDialogOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordDialogOpen(string errorMessage = "")
	{
		m_Dialogs.DisplayDialog(EJoinDialogState.PASSWORD_REQUIRED);
		
		// Input invoker actions 
		m_Dialogs.m_OnConfirm.Insert(JoinProcess_OnPasswordConfirm);
		//m_Dialogs.m_OnConfirm.Insert(JoinProcess_OnJoinSuccess);
		m_Dialogs.m_OnCancel.Insert(JoinProcess_PasswordClearInvokers);
		
		// Result invoker actions 
		m_CallbackPasswordCheck.m_OnSuccess.Insert(JoinProcess_OnPasswordCorrect);
 		m_CallbackPasswordCheck.m_OnFail.Insert(JoinProcess_OnPasswordWrong);
		m_CallbackPasswordCheck.m_OnTimeOut.Insert(JoinProcess_OnPasswordTimeout);
		
		// Display dialog message 
		SCR_EditboxDialogUi editDialog = SCR_EditboxDialogUi.Cast(m_Dialogs.GetCurrentDialog());
		if (editDialog)
		{
			//bool display = !errorMessage.IsEmpty();
			//editDialog.DisplayWarningMessage(display);
			
			editDialog.SetWarningMessage(errorMessage);
		}
	}
	
	protected ref RoomPasswordJoinParam m_passwordStruct = new RoomPasswordJoinParam;
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnPasswordConfirm()
	{
		SCR_EditboxDialogUi editboxDialog = SCR_EditboxDialogUi.Cast(m_Dialogs.GetCurrentDialog());
		
		// Check editbox dialog
		if (!editboxDialog)
		{
			PrintDebug("Password dialog is not using editbox dialog", "JoinProcess_OnPasswordConfirm");
			return;
		}
		
		// Get edit box value 
		string value = editboxDialog.GetEditbox().GetValue();
		
		// Try join with password 
		m_passwordStruct.SetPassword(value);
		JoinProcess_Join(m_RoomToJoin);
		//m_RoomToJoin.Join(m_CallbackPasswordCheck, m_passwordStruct);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordClearInvokers()
	{	
		m_Dialogs.m_OnConfirm.Clear();
		m_Dialogs.m_OnCancel.Clear();
		
		m_CallbackPasswordCheck.m_OnSuccess.Clear();
		m_CallbackPasswordCheck.m_OnFail.Clear();
		m_CallbackPasswordCheck.m_OnTimeOut.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnPasswordCorrect(ServerBrowserCallback callback)
	{
		JoinProcess_PasswordClearInvokers();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnPasswordWrong(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		JoinProcess_PasswordClearInvokers();
		JoinProcess_PasswordDialogOpen("#AR-Password_FailMessage");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnPasswordTimeout(ServerBrowserCallback callback)
	{
		JoinProcess_PasswordClearInvokers();
		JoinProcess_PasswordDialogOpen("#AR-Password_TimeoutMessage");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load data about room mods to check which data is client missing 
	//! Show state if mods are already loaded or wait for receiving mods data 
	protected void JoinProcess_LoadModContent(Room roomToJoin)
	{
		// Check references 
		if (!m_Dialogs || !m_ModsManager || !roomToJoin)
			return;
		
		// Check mods use privilege - UGC privilege 
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance(); 
		array<Dependency> deps = {};
		roomToJoin.AllItems(deps);
		
		if (!mgr.GetUgcPrivilege() && !deps.IsEmpty())
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING);
			
			// Negotatiate UGC privilege 
			mgr.m_OnUgcPrivilegeResult.Insert(Platform_OnUgcPrivilegeResult);
			mgr.NegotiateUgcPrivilegeAsync();
			return;
		}
		
		// Show state of mods if all loaded 
		JoinProcess_LoadModContentVisualize();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Platform_OnUgcPrivilegeResult(bool result)
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnUgcPrivilegeResult.Remove(Platform_OnUgcPrivilegeResult);
		if (result)
			JoinProcess_LoadModContentVisualize();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_LoadModContentVisualize()
	{
		// Show state of mods if all loaded 
		if (m_ModsManager.GetModsLoaded())
		{
			//SolveJoiningCases(); //TODO@wernerjak - solve joining cases function clearing 
			JoinProcess_CheckModContent();
			return;
		}
		
		// Show mods loading dialog 
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.CHECKING_CONTENT);
		
		// Set wait for loading 
		m_ModsManager.m_OnGettingAllDependecies.Insert(JoinProcess_CheckModContent);
		m_ModsManager.ReceiveRoomContentData(m_RoomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check state of room required mods on client
	//! Show missing mods data to download or move to next step
	//! Stop progess if room mods are restricted 
	protected void JoinProcess_CheckModContent()
	{
		Room roomToJoin = m_RoomToJoin;
		
		// Remove mods check actions 
		m_ModsManager.m_OnGettingAllDependecies.Remove(JoinProcess_CheckModContent);
		
		// Restricted content check 
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted(); 

		array <ref SCR_WorkshopItem> restricedMods = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.RESTRICTED);
		bool restricted = restricedMods.Count() > 0;
		
		// Stop join if there are restricted mods 
		if (restricted)
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.MOD_RESTRICTED);
			return;
		}
	
		// Split items 
		array<ref SCR_WorkshopItem> updated = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NO_PROBLEMS);
		array<ref SCR_WorkshopItem> outdated = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);
		
		// Join if room content is matching 
		if (outdated.IsEmpty())
		{
			//JoinProcess_Join(roomToJoin);
			JoinProcess_CheckRoomPasswordProtected(roomToJoin);
			return;	
		}	
		
		// Setup dialog with list of mods to update
		m_Dialogs.SetJoinRoom(roomToJoin);
		m_Dialogs.DisplayDialog(EJoinDialogState.MODS_TO_UPDATE);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call join if all clint pass through all check to join server 
	protected void JoinProcess_Join(Room roomToJoin)
	{
		// Join server 
		SetJoiningRoom(roomToJoin);
		//roomToJoin.Join(m_CallbackJoin, null);
		roomToJoin.Join(m_CallbackJoin, m_passwordStruct);
		
		// Add join callbacks 
		m_CallbackJoin.m_OnSuccess.Insert(JoinProcess_OnJoinSuccess);
		m_CallbackJoin.m_OnFail.Insert(JoinProcess_OnJoinFail);
		m_CallbackJoin.m_OnTimeOut.Insert(JoinProcess_OnJoinTimeout);

		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.JOIN);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room was successful
	//! Connect to the server 
	protected void JoinProcess_OnJoinSuccess(ServerBrowserCallback callback)
	{
		// Connect 
		m_RoomToJoin.Connect();
		
		// Save menu to reopen 
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu);
		
		// Cleanups - TODO@wernerjak - remove this cleaning to on function with other cleaning?
		m_RoomToJoin = null;
		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetBackendApi().GetWorkshop().Cleanup();
		
		// Clean invite 
		if (m_Lobby.GetInviteRoom())
		{
			m_Lobby.ClearInviteRoom();
			
			PrintDebug("Succesful join to invited server and cleaning the server!", "JoinProcess_OnJoinSuccess");
		}
		
		JoinProcess_CleanJoinCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room was successful
	//! Connect to the server 
	protected void JoinProcess_OnJoinFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		// Server is full 
		if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_IS_FULL)
		{
			if (m_Dialogs)
			{
				m_Dialogs.m_OnJoinRoomDemand.Insert(JoinProcess_Join);
				
				m_Dialogs.SetJoinRoom(m_JoiningRoom);
				m_Dialogs.DisplayDialog(EJoinDialogState.QUEUE_WAITING);
			}
			
			// Clear 
			JoinProcess_CleanJoinCallback();
			return;
		}
		
		// Fail dialog 
		if (m_Dialogs)
		{
			m_Dialogs.CloseCurrentDialog();
			m_Dialogs.DisplayJoinFail(apiCode);
		}
		
		// Try password again 
		if (m_RoomToJoin.PasswordProtected())
			JoinProcess_OnPasswordWrong(callback, code, restCode, apiCode);
		
		JoinProcess_CleanJoinCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room has timeout
	protected void JoinProcess_OnJoinTimeout(ServerBrowserCallback callback)
	{
		/*if (m_Dialogs)
			m_Dialogs.DisplayJoinFail();*/
		
		// TODO@wernerjak - add proper time out feedback dialog
		PrintDebug("join timeout in ServerBrowserMenuUI", "JoinProcess_OnJoinTimeout");
		
		JoinProcess_CleanJoinCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CleanJoinCallback()
	{
		m_CallbackJoin.m_OnSuccess.Remove(JoinProcess_OnJoinSuccess);

		m_CallbackJoin.m_OnFail.Remove(JoinProcess_OnJoinFail);
		m_CallbackJoin.m_OnTimeOut.Remove(JoinProcess_OnJoinTimeout);
		
		// Dialogs 
		m_Dialogs.m_OnJoinRoomDemand.Remove(JoinProcess_Join);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open joining dialog in default state 
	protected void SetupJoinDialogs()
	{
		// Check dialog 
		if (!m_Dialogs)
			return;
		
		// Setup basics 
		m_Dialogs.SetServerBrowser(this);
		m_Dialogs.SetModManager(m_ModsManager);
		
		// Invokers 
		m_Dialogs.m_OnDownloadComplete.Clear();
		m_Dialogs.m_OnDownloadComplete.Insert(OnDialogsHandlerDownloadComplete);

		m_aSearchCallbacks.Clear();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Privileges handling 
	//------------------------------------------------------------------------------------------------
	
	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;
	
	//------------------------------------------------------------------------------------------------
	//! Attempt to allow MP privilege
	protected void NegotiateMPPrivilegeAsync()
	{
		if (!m_CallbackGetPrivilege)
		{
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
			m_CallbackGetPrivilege.m_OnResult.Insert(OnMPPrivilegeResult);
		}
		
		GetGame().GetPlatformService().GetPrivilegeAsync(UserPrivilege.MULTIPLAYER_GAMEPLAY, m_CallbackGetPrivilege);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMPPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		// Sucessful
		if (privilege == UserPrivilege.MULTIPLAYER_GAMEPLAY && result == UserPrivilegeResult.ALLOWED)
		{
			OnActionRefresh();	
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// Custom debuging
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Custom debug print displayed with -scrDefine=SB_DEBUG argument
	protected void PrintDebug(string msg, string functionName = string.Empty)
	{
		#ifdef SB_DEBUG
		//Setup function format 
		string fncStr = string.Empty;
		if (functionName != string.Empty)
			fncStr = string.Format(" Fnc: %1()", functionName);
		
		// Display message 
		PrintFormat("[ServerBrowserMenuUI]%1 -- Msg: %2", fncStr, msg);
		#endif
	}
};

//------------------------------------------------------------------------------------------------
//! Overrided GetRoomsIds class to manipulation in script 
class SCR_GetRoomsIds extends GetRoomsIds
{
	protected ref array<string> roomIds = {};
	
	//------------------------------------------------------------------------------------------------
	void RegisterId(string id)
	{
		roomIds.Insert(id);
		RegV("roomIds");
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearIds()
	{
		roomIds.Clear();
	}
};

//------------------------------------------------------------------------------------------------
enum ESBWidgetFocus
{
	SERVER_LIST = 0,
	FILTERING = 1,
	SORTING = 2,
	SEARCH = 3
};

//------------------------------------------------------------------------------------------------
enum ESBServerFeedback
{
	LOADING_SERVERS,
	NO_SERVERS,
	BACKED_ERROR,
	SERVER_BROWSER
};