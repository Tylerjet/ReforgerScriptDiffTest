/*!
Class for handling server browser dialogs.
Server browser use multiple dialogs in joining process for checking servers, mod content and restrictinos.
*/

//------------------------------------------------------------------------------------------------
class SCR_ServerBrowserDialogManager
{
	// Resources
	const ResourceName CONFIG_DIALOGS = "{471EFCF445C3E9C6}Configs/ServerBrowser/JoiningDialogs.conf";
	const ResourceName CONFIG_DIALOGS_ERROR = "{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf";

	// Dialog tags references
	const string TAG_SEARCHING_SERVER 			= "SEARCHING_SERVER";
	const string TAG_JOIN 						= "JOIN";
	const string TAG_REJOIN 					= "REJOIN";
	const string TAG_SERVER_NOT_FOUND 			= "SERVER_NOT_FOUND";
	const string TAG_VERSION_MISMATCH 			= "VERSION_MISMATCH";
	const string TAG_CHECKING_CONTENT 			= "CHECKING_CONTENT";
	const string TAG_MOD_UGC_PRIVILEGE_MISSING 	= "MOD_UGC_PRIVILEGE_MISSING";
	const string TAG_MODS_DOWNLOADING 			= "MODS_DOWNLOADING";
	const string TAG_QUEUE_WAITING 				= "QUEUE_WAITING";
	const string TAG_PASSWORD_REQUIRED 			= "PASSWORD_REQUIRED";
	const string TAG_BACKEND_TIMEOUT			= "BACKEND_TIMEOUT";
	const string TAG_KICK_DEFAULT 				= "DEFAULT_ERROR";

	protected const int MAX_AUTO_REJOINS = 3;

	// References
	protected Room m_JoinRoom;
	protected SCR_RoomModsManager m_ModManager;
	protected ServerBrowserMenuUI m_ServerBrowser;

	// States
	protected bool m_bIsOpen = false;
	protected int m_iOpenDialogCount = 0;
	protected EJoinDialogState m_iDisplayState;
	protected SCR_ConfigurableDialogUi m_CurrentDialog;
	protected SCR_ConfigurableDialogUi m_CurrentKickDialog;

	// Join dialog widget handling
	const string WIDGET_IMAGE_ICON = "ImgIcon";
	const string WIDGET_TXT_SUBMSG = "TxtSubMsg";
	const string WIDGET_LOADING = "LoadingCircle0";

	const string WIDGET_PROGRESS = "VProgress";
	const string WIDGET_TXT_PROGRESS = "TxtProgress";
	const string WIDGET_PROGRESS_BAR = "ProgressBar";

	const string WIDGET_BUTTON_ADDITIONAL = "Additional";
	const string ERROR_TAG_DEFAULT = "JOIN_FAILED";

	protected int m_iDownloadedCount = 0;
	protected ref array<ref SCR_WorkshopItemAction> m_JoinDownloadActions = {};

	// Invokers
	ref ScriptInvoker m_OnConfirm = new ScriptInvoker();
	ref ScriptInvoker m_OnCancel = new ScriptInvoker();
	ref ScriptInvoker m_OnDialogClose = new ScriptInvoker();

	ref ScriptInvoker m_OnDownloadComplete = new ScriptInvoker();
	ref ScriptInvoker<Room> m_OnJoinRoomDemand = new ScriptInvoker();
	protected ref ScriptInvoker<> Event_OnRejoinTimerOver;
	protected ref ScriptInvoker Event_OnCloseAll = new ScriptInvoker();


	//------------------------------------------------------------------------------------------------
	// Public functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Display dialog by state and setup current state
	void DisplayDialog(EJoinDialogState state)
	{
		// Set
		m_iDisplayState = state;

		//m_bIsOpen = true;
		if (m_CurrentDialog)
			m_CurrentDialog.Close();

		// Visual set
		switch (state)
		{
			// Searching server
			case EJoinDialogState.SEARCHING_SERVER:
				SetDialogByTag(TAG_SEARCHING_SERVER);
				break;

			// Joining
			case EJoinDialogState.JOIN:
				SetDialogByTag(TAG_JOIN);
				break;

			// Joining
			case EJoinDialogState.REJOIN:
			{
				SetDialogByTag(TAG_REJOIN);
				break;
			}

			// Server by given parameters wasn't found
			case EJoinDialogState.SERVER_NOT_FOUND:
				SetDialogByTag(TAG_SERVER_NOT_FOUND);
				break;

			// Wrong version restriction
			case EJoinDialogState.VERSION_MISMATCH:
			{
				SetDialogByTag(TAG_VERSION_MISMATCH);

				// Display version difference text
				string clientV = GetGame().GetBuildVersion();
				string RoomV = m_JoinRoom.GameVersion();

				string msg = string.Format("%1: %2 \n%3: %4", "#AR-ServerBrowser_Server", RoomV, "#AR-Editor_AttributeCategory_GameSettings_Name", clientV);
				m_CurrentDialog.SetMessage(msg);
				break;
			}

			// Checking mod content
			case EJoinDialogState.CHECKING_CONTENT:
				SetDialogByTag(TAG_CHECKING_CONTENT);
				break;

			// Mod to update
			case EJoinDialogState.MODS_TO_UPDATE:
				//DisplayModsToUpdate();
				break;

			// Restricted mods - banned & reported
			case EJoinDialogState.MOD_RESTRICTED:
				DisplayRestricetedModList();
				break;

			// Download progress
			case EJoinDialogState.MODS_DOWNLOADING:
				DisplayModsDownloading();
				break;

			// Cleint can't access user generated content
			case EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING:
				SetDialogByTag(TAG_MOD_UGC_PRIVILEGE_MISSING);
				break;

			// Queue waiting
			case EJoinDialogState.QUEUE_WAITING:
				DisplayWaitingQueue();
				break;

			// Password input dialog
			case EJoinDialogState.PASSWORD_REQUIRED:
			{
				DisplayPasswordRequired();
				break;
			}

			// Password input dialog
			case EJoinDialogState.BACKEND_TIMEOUT:
			{
				SetDialogByTag(TAG_BACKEND_TIMEOUT);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create and setup error dialog when ending multiplayer game
	void DisplayKickErrorDialog(string tag, string group, string strDetail)
	{
		SCR_ConfigurableDialogUi dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, tag);

		// Use group as fallback if no dialog found
		if (!dialogUi)
		{
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, group);
		}

		// Show default error if tag is not found
		if (!dialogUi)
		{
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, TAG_KICK_DEFAULT);
		}

		m_CurrentKickDialog = dialogUi;

		if (dialogUi)
		{
			// Set error details
			SCR_ErrorDialog errorDialog = SCR_ErrorDialog.Cast(dialogUi.GetRootWidget().FindHandler(SCR_ErrorDialog));
			if (errorDialog)
			{
				errorDialog.SetErrorDetail(strDetail);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void DisplayReconnectDialog(Room roomToJoin, int time, string strDetail = "")
	{
		// Check dialog
		if (!m_CurrentKickDialog)
		{
			//PrintDebug("No kick dialog", "DisplayReconnectDialog");
			return;
		}

		// Check kick preset
		SCR_KickDialogUiPreset kickPreset = SCR_KickDialogUiPreset.Cast(m_CurrentKickDialog.GetDialogPreset());
		if (!kickPreset)
		{
			//PrintDebug("Missing dialog kick preset", "DisplayReconnectDialog");
			return;
		}

		m_CurrentKickDialog.Close();

		// Set reconnect
		if (!kickPreset.m_ReconnectPreset)
		{
			//PrintDebug("Missing dialog reconnect preset", "DisplayReconnectDialog");
			return;
		}

		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateByPreset(
			kickPreset.m_ReconnectPreset);

		m_CurrentKickDialog = dialog;

		// Set Messages
		dialog.SetMessage(kickPreset.m_sMessage);

		SCR_RejoinDialog rejoinDialog = SCR_RejoinDialog.Cast(m_CurrentKickDialog.GetRootWidget().FindHandler(SCR_RejoinDialog));
		if (rejoinDialog)
		{
			rejoinDialog.SetErrorDetail(strDetail);

			string errorStr = kickPreset.m_sMessage;
			rejoinDialog.SetErrorMessage(errorStr);

			// Check rejoin attempt
			string strAttempt = GameSessionStorage.s_Data["m_iRejoinAttempt"];
			int attempt = strAttempt.ToInt();

			if (attempt <= MAX_AUTO_REJOINS)
			{
				errorStr += "\n" + "#AR-ServerBrowser_JoinMessageDefault";
				rejoinDialog.SetErrorMessage(errorStr);

				dialog.SetMessage(errorStr + " " + rejoinDialog.GetTimer());

				// Setup timer
				rejoinDialog.GetEventOnTimerChanged().Insert(OnDialogTimerChange);

				rejoinDialog.ShowLoading(true);
				rejoinDialog.SetTimer(time);
				rejoinDialog.RunTimer(true);
			}
			else
			{
				dialog.SetMessage(errorStr);

				// Block rejoin
				rejoinDialog.ShowLoading(false);
				m_CurrentKickDialog.FindButton("confirm").SetEnabled(false);
			}
		}


		// Setup dialog
		if (dialog)
		{
			// Title
			dialog.SetTitle(kickPreset.m_sTitle);

			// Reconnect button
			dialog.m_OnConfirm.Insert(OnDialogConfirm);
			dialog.m_OnCancel.Insert(OnDialogCancel);
			dialog.m_OnClose.Insert(OnDialogClose);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnRejoinTimerOver()
	{
		if (Event_OnRejoinTimerOver)
			Event_OnRejoinTimerOver.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnRejoinTimerOver()
	{
		if (!Event_OnRejoinTimerOver)
			Event_OnRejoinTimerOver = new ScriptInvoker();

		return Event_OnRejoinTimerOver;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogTimerChange(SCR_RejoinDialog dialog, int time)
	{
		m_CurrentKickDialog.SetMessage(dialog.GetErrorMessage() + " " + time);

		if (time == 0)
		{
			InvokeEventOnRejoinTimerOver();
			m_CurrentKickDialog.Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetDialogMessage(string msg)
	{
		if (!m_CurrentDialog)
			return;

		m_CurrentDialog.SetMessage(msg);
	}

	//------------------------------------------------------------------------------------------------
	void CloseCurrentDialog()
	{
		if (m_CurrentDialog)
			m_CurrentDialog.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Quick short code to open dialog by tag and cache it
	protected void SetDialogByTag(string tag, SCR_ConfigurableDialogUi dialog = null)
	{
		// Check dialog resource
		if (CONFIG_DIALOGS.IsEmpty())
			return;

		// Remove invokers actions from old dialog
		if (m_CurrentDialog)
		{
			m_CurrentDialog.m_OnConfirm.Remove(OnDialogConfirm);
		}

		// Create dialog
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, tag, dialog);
		m_iOpenDialogCount++;
		m_bIsOpen = true;

		// Add invoker actions to current dialog
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		m_CurrentDialog.m_OnClose.Insert(OnDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogConfirm()
	{
		m_OnConfirm.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogCancel()
	{
		m_OnCancel.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnCloseAll()
	{
		if (!Event_OnCloseAll)
			Event_OnCloseAll = new ScriptInvoker();

		Event_OnCloseAll.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCloseAll()
	{
		if (!Event_OnCloseAll)
			Event_OnCloseAll = new ScriptInvoker();

		return Event_OnCloseAll;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		m_OnDialogClose.Invoke();

		if (m_CurrentDialog == dialog)
			InvokeOnCloseAll();
	}


	//------------------------------------------------------------------------------------------------
	// Spefic dialog handling
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Show join fail message in dialog
	void DisplayJoinFail(EApiCode apiError)
	{
		string errorTag = ERROR_TAG_DEFAULT;

		// TODO@wernerjak - setup dialogs

		switch (apiError)
		{
			case EApiCode.EACODE_ERROR_P2P_USER_JOIN_BAN: errorTag = "JOIN_FAILED_BAN"; break;
			case EApiCode.EACODE_ERROR_DS_USER_JOIN_BAN: errorTag = "JOIN_FAILED_BAN"; break;
			case EApiCode.EACODE_ERROR_USER_IS_BANNED_FROM_SHARED_GAME: errorTag = "JOIN_FAILED_BAN"; break;
			//case EApiCode.EACODE_ERROR_PLAYER_IS_BANNED: errorTag = "JOIN_FAILED_BAN"; break;
			//case EApiCode.EACODE_ERROR_MAINTENANCE_IN_PROGRESS: errorTag = "JOIN_FAILED_MAITANANCE"; break;
			//case EApiCode.EACODE_ERROR_MP_ROOM_IS_NOT_JOINABLE: errorTag = "JOIN_FAILED_NOT_JOINABLE"; break;
		}

		/*
		Player banned from session


		*/

		SetDialogByTag(errorTag);

		// Show additional message
		if (m_CurrentDialog)
		{
			SCR_ErrorDialog errorDialog = SCR_ErrorDialog.Cast(m_CurrentDialog.GetRootWidget().FindHandler(SCR_ErrorDialog));
			if (errorDialog)
			{
				string strApiError = typename.EnumToString(EApiCode, apiError);
				errorDialog.SetErrorDetail("#AR-Workshop_Error: " + strApiError);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Display list of restricted mods
	protected void DisplayRestricetedModList()
	{
		array<ref SCR_WorkshopItem> items = m_ModManager.GetRoomItemsScripted();
		m_CurrentDialog = SCR_AddonListDialog.CreateRestrictedAddonsJoinServer(items);

		SCR_ReportedAddonsDialog reportedDialog = SCR_ReportedAddonsDialog.Cast(m_CurrentDialog);

		// Handle cancel reports done
		if (reportedDialog)
			reportedDialog.GetOnAllReportsCanceled().Insert(OnAllReportsCanceled);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when all reports from dialog are cancled to clear invoker actions and display download dialog
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
	}

	//------------------------------------------------------------------------------------------------
	//! Show process of mods to donwlaoad
	protected void DisplayModsDownloading()
	{
		DialogUI dialog = SCR_DownloadManager_Dialog.Create();

		SCR_DownloadManager.GetInstance().m_OnDownloadQueueCompleted.Insert(OnDownloadingDone);
		dialog.m_OnCancel.Insert(OnDownloadingDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadingDone()
	{
		// Check if any of required addon is not failed
		array<ref SCR_WorkshopItem> required = m_ModManager.GetRoomItemsScripted();

		for (int i = 0, count = required.Count(); i < count; i++)
		{
			// Is offline
			if (!required[i].GetOffline())
				return;
		}

		m_OnDownloadComplete.Invoke(m_JoinRoom);
		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Remove(OnDownloadActionFailed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadingDialogClose()
	{
		// Display download cancel dialog
		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, "CANCEL_DOWNLOAD");
		dialog.m_OnConfirm.Insert(OnCancelDownloadDialogConfirm);
		dialog.m_OnCancel.Insert(OnCancelDownloadDialogCancel);

		// Cancel joining process
		SCR_DownloadManager.GetInstance().m_OnDownloadQueueCompleted.Remove(OnDownloadingDone);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		SCR_DownloadManager.GetInstance().EndAllDownloads();

		// Clear
		dialog.m_OnConfirm.Remove(OnCancelDownloadDialogConfirm);
		dialog.m_OnCancel.Remove(OnCancelDownloadDialogCancel);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadDialogCancel(SCR_ConfigurableDialogUi dialog)
	{
		// Clear
		dialog.m_OnConfirm.Remove(OnCancelDownloadDialogConfirm);
		dialog.m_OnCancel.Remove(OnCancelDownloadDialogCancel);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadActionFailed(notnull SCR_WorkshopItemAction action)
	{
		CloseCurrentDialog();

		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Remove(OnDownloadActionFailed);
	}

	//------------------------------------------------------------------------------------------------
	//! Display waiting queue when server is full
	//! Show current state of players
	protected void DisplayWaitingQueue()
	{
		SetDialogByTag(TAG_QUEUE_WAITING);
		UpdateWaitingQueue();

		// Set state text
		TextWidget txtStateTitle = TextWidget.Cast(m_CurrentDialog.GetRootWidget().FindAnyWidget("TxtStateTitle"));
		if (txtStateTitle)
			txtStateTitle.SetText("#AR-PlayerList_Header");

		// Set action
		m_CurrentDialog.m_OnConfirm.Insert(OnWaitingQueueConfirm);

		#ifdef SB_DEBUG
		Print("[SCR_ServerBrowserDialogManager] displaying queue waiting dialog");
		#endif
	}


	//------------------------------------------------------------------------------------------------
	//! Dispaly dialog for password insert whenever oassword is required
	protected void DisplayPasswordRequired()
	{
		SCR_EditboxDialogUi editboxDialog = new SCR_EditboxDialogUi();
		SetDialogByTag(TAG_PASSWORD_REQUIRED, editboxDialog);

		editboxDialog.m_OnWriteModeLeave.Insert(OnPasswordEditboxChanged);
		editboxDialog.FindButton("confirm").SetEnabled(false);
		editboxDialog.FindButton("cancel").SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this whenever password dialog editbox is being edited
	protected void OnPasswordEditboxChanged(string text)
	{
		if (!m_CurrentDialog || !SCR_EditboxDialogUi.Cast(m_CurrentDialog))
			return;

		SCR_NavigationButtonComponent navButton = m_CurrentDialog.FindButton("confirm");
		if (navButton)
			navButton.SetEnabled(!text.IsEmpty());

		navButton = m_CurrentDialog.FindButton("cancel");
		if (navButton)
			navButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this on server browser resfresh
	protected void OnServerBrowserAutoRefresh()
	{
		if (m_iDisplayState == EJoinDialogState.QUEUE_WAITING)
			UpdateWaitingQueue();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWaitingQueueConfirm(SCR_ConfigurableDialogUi dialog)
	{
		if (m_ServerBrowser)
		{
			m_OnJoinRoomDemand.Invoke(m_JoinRoom);
			//m_ServerBrowser.JoinRoom(m_JoinRoom);
		}


		m_CurrentDialog.m_OnConfirm.Remove(OnWaitingQueueConfirm);
	}

	//------------------------------------------------------------------------------------------------
	//! Update wating queue with acutal player count
	protected void UpdateWaitingQueue()
	{
		// Check room
		if (!m_JoinRoom)
			return;

		// Gte player state
		int count = m_JoinRoom.PlayerCount();
		int limit = m_JoinRoom.PlayerLimit();

		if (!m_CurrentDialog)
			return;

		// Set text
		TextWidget txtState = TextWidget.Cast(m_CurrentDialog.GetRootWidget().FindAnyWidget("TxtState"));
		if (txtState)
			txtState.SetText(count.ToString() + "/" + limit.ToString());

		// Setup button
		SCR_NavigationButtonComponent btnConfirm = m_CurrentDialog.FindButton("confirm");
		if (btnConfirm)
		{
			bool enable = limit - count > 0;
			btnConfirm.SetEnabled(enable);
		}
	}

	//! DECOUPLED DIALOGS
	//------------------------------------------------------------------------------------------------
	MultiplayerDialogUI CreateManualJoinDialog()
	{
		MultiplayerDialogUI multiplayerDialogUI = new MultiplayerDialogUI();
		MultiplayerDialogUI dialog = MultiplayerDialogUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, "MANUAL_CONNECT", multiplayerDialogUI));
		m_CurrentDialog = dialog;

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ServerDetailsDialog CreateServerDetailsDialog(Room room, array<ref SCR_WorkshopItem> items, ScriptInvoker onFavoritesResponse = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.CreateServerDetailsDialog(room, items, "SERVER_DETAILS", CONFIG_DIALOGS, onFavoritesResponse);
		m_CurrentDialog = dialog;

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	void FillRoomDetailsMods(array<ref SCR_WorkshopItem> items, SCR_RoomModsManager modsManager = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		dialog.FillModList(items, modsManager);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRoomDetailsScenarioImage(MissionWorkshopItem scenario)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		if(dialog)
			dialog.SetScenarioImage(scenario);
	}

	//------------------------------------------------------------------------------------------------
	// Get & Set API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	Room GetJoinRoom()
	{
		return m_JoinRoom;
	}

	//------------------------------------------------------------------------------------------------
	EJoinDialogState GetDisplayState()
	{
		return m_iDisplayState;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUi GetCurrentDialog()
	{
		return m_CurrentDialog;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDialogOpen()
	{
		return GetCurrentDialog();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUi GetCurrentKickDialog()
	{
		return m_CurrentKickDialog;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bIsOpen;
	}

	//------------------------------------------------------------------------------------------------
	void SetJoinRoom(Room room)
	{
		m_JoinRoom = room;
	}

	//------------------------------------------------------------------------------------------------
	void SetServerBrowser(ServerBrowserMenuUI serverBrowser)
	{
		m_ServerBrowser = serverBrowser;

		if (m_ServerBrowser)
		{
			m_ServerBrowser.m_OnAutoRefresh.Remove(OnServerBrowserAutoRefresh);
			m_ServerBrowser.m_OnAutoRefresh.Insert(OnServerBrowserAutoRefresh);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetModManager(SCR_RoomModsManager modManager)
	{
		m_ModManager = modManager;
	}

};

//------------------------------------------------------------------------------------------------
//! Enum for tracking current state of joining process
enum EJoinDialogState
{
	// Basic states
	SEARCHING_SERVER, // Basic msg dialog with loading
	JOIN, // Basic msg dialog with loading
	REJOIN, // Rejoin dialog if client is kicked

	// Basic issues states
	SERVER_NOT_FOUND, // Basic error msg dialog
	VERSION_MISMATCH, // Basic error msg dialog
	JOIN_FAILED,

	// Content states
	CHECKING_CONTENT, // Basic msg dialog with loading
	MODS_TO_UPDATE, // Download confirm dialog
	MODS_DOWNLOADING, // Progress bar dialog
	MOD_RESTRICTED, // Basic error msg dialog
	MOD_UGC_PRIVILEGE_MISSING, // Client can't use user generated content

	// Additional actions states
	QUEUE_WAITING, // Dialog with player count
	PASSWORD_REQUIRED, // Editbox dialog

	// Issues
	SERVICE_DOWN, // Backend service is not available
	SERVER_DOWN, // Joined server is not running
	BACKEND_TIMEOUT,
};
