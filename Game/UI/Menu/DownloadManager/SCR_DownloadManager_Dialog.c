/*
Main dialog of the download manager, which is typically opened from the download manager panel.

It lists all current and previous downloads.

!!! For the menu to work, it needs a SCR_DownloadManager_Entity to be placed in the world.
This is required because the menu can be opened after any of the downloads have been started,
but by that time we must store the download references somewhere.
*/
class SCR_DownloadManager_Dialog : SCR_TabDialog
{
	protected const string STATE_DOWNLOADING = "#AR-Workshop_TabName_Downloaded";
	protected const string STATE_ALL_PAUSE = "#AR-DownloadManager_State_AllDownloadsPaused";
	protected const string STATE_NO_ACTIVE_DOWNLOADS = "#AR-DownloadManager_State_NoActiveDownloads";
	
	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{FB196DBC0ABA6AE4}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManagerEntry.layout";
	protected const string DOWNLOAD_SUMMARY_FORMAT = "<color rgba=\"226, 167, 79, 255\">%1</color> / <color rgba=\"226, 167, 79, 255\">%2</color>";
	
	protected const string FAILED_ADDON_FORMAT = "- %1 \n"; 
	protected const string FAILED_ADDON_LIST_DIALOG = "failed_dialogs_list";
	
	protected ref SCR_DownloadManagerDialogWidgets m_Widgets = new SCR_DownloadManagerDialogWidgets();
	
	protected SCR_ConfigurableDialogUi m_DownloadFailDialog;
	protected SCR_MessageDialogContent m_DownloadFailDialogContent;
	
	protected ref SCR_DownloadManagerListComponent m_ActiveDownloads;
	protected ref SCR_DownloadManagerListComponent m_HistoryDownloads;
	
	protected static bool m_bOpened = false;
	
	//-----------------------------------------------------------------------------------------------
	static SCR_DownloadManager_Dialog Create()
	{	
		if (m_bOpened)
			return null;
		
		MenuBase menu = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.DownloadManagerDialog);
		
		return SCR_DownloadManager_Dialog.Cast(menu);
	}
	
	
	//------------------------------------------------------------------------------------------
	//! Cretes a navigation button to open download manager
	//! Use this in your menu: SCR_DownloadManager_Dialog.CreateNavigationButton(this);
	static void CreateNavigationButton2(notnull SCR_SuperMenuBase superMenu)
	{
		SCR_NavigationButtonComponent button = superMenu.AddNavigationButton("MenuDownloadManager", "#AR-DownloadManager_ButtonDownloads", rightFooter: false);
		button.m_OnActivated.Insert(SCR_DownloadManager_Dialog.Create);
	}
	
	//------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{	
		super.OnMenuOpen();
		
		InitWidgets();
		
		SCR_SuperMenuComponent superMenu = SCR_SuperMenuComponent.Cast(GetRootWidget().FindHandler(SCR_SuperMenuComponent));
		
		// Setup submenus 
		m_ActiveDownloads = SCR_DownloadManagerListComponent.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(0).m_wTab.FindHandler(SCR_DownloadManagerListComponent));
		
		m_ActiveDownloads.Init(superMenu);
		
		m_HistoryDownloads = SCR_DownloadManagerListComponent.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(1).m_wTab.FindHandler(SCR_DownloadManagerListComponent));
		
		m_HistoryDownloads.Init(superMenu);
		m_HistoryDownloads.ShowPauseResumeAllButton(false);
		
		InitList();
		
		SCR_DownloadManager.GetInstance().m_OnNewDownload.Insert(OnNewDownload);
		SCR_DownloadManager.GetInstance().m_OnDownloadComplete.Insert(OnDownloadComplete);
		SCR_DownloadManager.GetInstance().m_OnDownloadFailed.Insert(OnDownloadFailed);
		SCR_DownloadManager.GetInstance().m_OnDownloadCanceled.Insert(OnDownloadCanceled);
			
		m_bOpened = true;
	}
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{	
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
		
		// Clear all recent failed downloads
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
		
		m_bOpened = false;
	}
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (!mgr)
			return;
		int nCompleted, nTotal;
			mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		SCR_WorkshopItemActionDownload dlAction = GetSelectedAction();
		bool pauseCancelEnabled;	// True when pause or cancel buttons should be enabled
		if (dlAction)
		{
			if (!dlAction.IsFailed() && !dlAction.IsCompleted() && !dlAction.IsCanceled())
				pauseCancelEnabled = true;
		}
		
		// Pause All / Resume All button label
		bool paused = mgr.GetDownloadsPaused();
		string pauseAllButtonLabel = "#AR-DownloadManager_ButtonPauseAll";
		if (paused)
			pauseAllButtonLabel = "#AR-DownloadManager_ButtonResumeAll";
		
		// Pause button
		string pauseButtonLabel = "#AR-DownloadManager_ButtonPause";
		if (dlAction)
		{
			if (dlAction.IsPaused() || dlAction.IsInactive())
				pauseButtonLabel = "#AR-DownloadManager_ButtonResume";
		}
		
		// Show downlaoding speed 
		UpdateProgressWidgets();
		UpdateDownloadingSpeedWidget(m_ActiveDownloads.HasContent());

		if (m_ActiveDownloads.GetShown())
			m_ActiveDownloads.ShowPauseResumeAllButton(nCompleted != nTotal);
		else
		{
			m_ActiveDownloads.ShowPauseResumeAllButton(false);
			m_HistoryDownloads.ShowPauseResumeAllButton(false);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateProgressWidgets()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		string downloadStateText;
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		if (nTotal > 0 && !mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate(STATE_DOWNLOADING), nCompleted, nTotal);
		else if (nTotal > 0 && mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate(STATE_ALL_PAUSE), nCompleted, nTotal);
		else if (mgr.GetDownloadsPaused())
			downloadStateText = STATE_ALL_PAUSE;
		else
			downloadStateText = STATE_NO_ACTIVE_DOWNLOADS;
		
		m_Widgets.m_StateText.SetText(downloadStateText);
		
		m_Widgets.m_Overlay.SetVisible(m_ActiveDownloads.HasContent());
		m_Widgets.m_DownloadSummaryText.SetVisible(m_ActiveDownloads.HasContent());
		
		// Progress bar, progress text
		if (nTotal > 0)
		{
			// Get total progress of all downloads
			array<ref SCR_WorkshopItemActionDownload> downloadQueue = mgr.GetDownloadQueue();
			float progress = SCR_DownloadManager.GetDownloadActionsProgress(downloadQueue);
			
			// Progress bar
			m_Widgets.m_ProgressBarComponent.SetValue(progress);
			
			// Progress percent text
			string progressText = WidgetManager.Translate("#AR-ValueUnit_Percentage", Math.Floor(progress * 100.0));
			m_Widgets.m_ProgressText.SetText(progressText);
			
			// Display total size 
			float downloadTotalSize = SCR_DownloadManager.GetInstance().GetDownloadQueueSize();
			
			if (downloadTotalSize > 0)
			{
				float downloadDoneSize =  SCR_DownloadManager.GetInstance().GetDownloadedSize();

				string downloadTotalSizeStr = SCR_ByteFormat.GetReadableSize(downloadTotalSize);
				string downloadDoneSizeStr = SCR_ByteFormat.ContentDownloadFormat(downloadDoneSize);
				
				m_Widgets.m_DownloadSummaryText.SetText(string.Format(DOWNLOAD_SUMMARY_FORMAT, downloadDoneSizeStr, downloadTotalSizeStr));
			}
		}
		else
		{
			m_Widgets.m_ProgressBarComponent.SetValue(0);
		}
		
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateDownloadingSpeedWidget(bool display)
	{
		m_Widgets.m_DownloadSpeed.SetVisible(display);
		if (!display)
			return;
		
		string unit = " KB/s";
		float speed = GetGame().GetRestApi().GetDownloadTrafficKBPerS();
		
		// Faster speed
		if (speed > SCR_ByteFormat.BYTE_UNIT_SIZE)
		{
			speed /= SCR_ByteFormat.BYTE_UNIT_SIZE;
			unit = " MB/s";
		}
			
		// Show text
		m_Widgets.m_DownloadSpeed.SetText(Math.Round(speed).ToString() + unit);
	}
	
	//------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		m_Widgets.Init(GetRootWidget());
		GetGame().GetInputManager().AddActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
	}
	
	//-----------------------------------------------------------------------------------------------
	// Init list of downloads
	protected void InitList()
	{
		array<ref SCR_DownloadManager_Entry> downloads = new array<ref SCR_DownloadManager_Entry>;
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		array<ref SCR_WorkshopItemActionDownload> failedDownloads = SCR_DownloadManager.GetInstance().GetFailedDownloads();
		
		for (int i = 0, count = downloads.Count(); i < count; i++)
		{
			SCR_WorkshopItemActionDownload action = downloads[i].m_Action;
			
			if (SCR_DownloadManagerEntry.DownloadActionState(action) == EDownloadManagerActionState.RUNNING)
			{
				m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
			}
			else
			{
				// Show recent fails in active
				if (failedDownloads.Find(action) != -1)
					m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
				else
					m_HistoryDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected SCR_WorkshopItemActionDownload GetSelectedAction()
	{
		Widget w = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (!w)
			return null;
		
		SCR_DownloadManagerEntry entry  = SCR_DownloadManagerEntry.Cast(w.FindHandler(SCR_DownloadManagerEntry)); 
		if (!entry)
			return null;
		
		return entry.GetDownloadAction();
	}

	//------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------
	protected void OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		// Remove same item in history 
		SCR_DownloadManagerEntry sameEntry = m_HistoryDownloads.EntryWithItem(item);
		if (sameEntry)
			m_HistoryDownloads.RemoveEntry(sameEntry);
		
		// Check same item in active
		sameEntry = m_ActiveDownloads.EntryWithItem(item);
		
		if (sameEntry)
		{
			m_ActiveDownloads.ChangeEntryCategory(sameEntry.GetRootWidget(), EDownloadManagerActionState.RUNNING);
			sameEntry.InitForDownloadAction(item, item.GetDownloadAction());
		}
		else
		{
			// Add action on activate in order to have target version
			action.m_OnActivated.Insert(OnNewActionActivate);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnNewActionActivate(SCR_WorkshopItemActionDownload action)
	{
		m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
		action.m_OnActivated.Remove(OnNewActionActivate);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnDownloadComplete(SCR_WorkshopItemActionDownload action)
	{
		// Move to downloaded 
		SCR_DownloadManagerEntry entry  = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.DOWNLOADED);
	}
	
	//------------------------------------------------------------------------------------------
	//! Call when download are not running
	//! Move view to failed downlaods and show dialog with listed downloads
	protected void OnDownloadFailed(SCR_WorkshopItemActionDownload action)
	{
		array<ref SCR_DownloadManager_Entry> downloads = {};
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		SCR_DownloadManagerEntry entry = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.FAILED);
		
		// Open active and scroll up 
		m_SuperMenu.GetTabView().ShowTab(0, false);
		m_ActiveDownloads.ScrollTop();
		
		// Create warnign dialog 	
		if (!m_DownloadFailDialog)
		{
			m_DownloadFailDialog = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, FAILED_ADDON_LIST_DIALOG);
			m_DownloadFailDialog.m_OnClose.Insert(OnDownloadFailDialogClose);
			
			// Setup message 
			Widget contentWidget = m_DownloadFailDialog.GetContentLayoutRoot(m_DownloadFailDialog.GetRootWidget());
			m_DownloadFailDialogContent = SCR_MessageDialogContent.Cast(contentWidget.FindHandler(SCR_MessageDialogContent));
			
			m_DownloadFailDialogContent.SetMessage("\n");
		}

		// Add failed downloads to list 
		if (m_DownloadFailDialogContent)
			m_DownloadFailDialogContent.SetMessage(m_DownloadFailDialogContent.GetMessage() + string.Format(FAILED_ADDON_FORMAT, action.GetAddonName()));
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnDownloadFailDialogClose()
	{
		m_DownloadFailDialog.m_OnClose.Remove(OnDownloadFailDialogClose);
		m_DownloadFailDialog = null;
		m_DownloadFailDialogContent = null
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnDownloadCanceled(SCR_WorkshopItemActionDownload action)
	{
		SCR_DownloadManagerEntry entry = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.FAILED);
	}
	
	//-----------------------------------------------------------
	protected void OnBackButton()
	{
		m_OnCancel.Invoke();
		Close();
	}
	
	//-----------------------------------------------------------
	protected void OnPauseAllButton()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (!mgr)
			return;
		
		bool paused = mgr.GetDownloadsPaused();
		mgr.SetDownloadsPaused(!paused);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPauseButton()
	{
		auto action = this.GetSelectedAction();
		
		if (!action)
			return;
		
		if (action.IsPaused())
			action.Resume();
		else if (action.IsInactive())
			action.Activate();
		else
			action.Pause();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnCancelButton()
	{
		auto action = this.GetSelectedAction();
		
		if (!action)
			return;
		
		action.Cancel();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnAutoenableButton()
	{
		SCR_WorkshopSettings settings = SCR_WorkshopSettings.Get();
		//settings.m_bAutoEnableDownloadedAddons = m_Widgets.m_AutoenableAddonsButtonComponent.GetToggled();
		SCR_WorkshopSettings.Save(settings);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SCR_DownloadManager_Dialog()
	{
		m_bOpened = false;
	}
	
	//-----------------------------------------------------------------------------------------------
	void ~SCR_DownloadManager_Dialog()
	{
		m_bOpened = false;
	}
};

/*!
A mini download manager dialog. It only has a few actions and a basic download progress indicator.
*/

class SCR_DownloadManager_MiniDialog : SCR_ConfigurableDialogUi
{
	SCR_NavigationButtonComponent m_PauseResumeButton;
	SCR_NavigationButtonComponent m_CancelButton;
	
	//------------------------------------------------------------------------------------------
	static SCR_DownloadManager_MiniDialog Create()
	{
		SCR_DownloadManager_MiniDialog dlg = new SCR_DownloadManager_MiniDialog();
		SCR_DownloadManager_MiniDialog.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, "download_manager_mini", dlg);
		return dlg;
	}
	
	
	
	//------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		m_PauseResumeButton = this.FindButton("download_pause_resume");
		m_CancelButton = this.FindButton("download_cancel");
		
		m_PauseResumeButton.m_OnActivated.Insert(OnPauseResumeButton);
		m_CancelButton.m_OnActivated.Insert(OnCancelButton);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (!mgr)
			return;
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		// Pause / Resume button label
		bool paused = mgr.GetDownloadsPaused();
		string pauseButtonLabel = "#AR-DownloadManager_ButtonPause";
		if (paused)
			pauseButtonLabel = "#AR-DownloadManager_ButtonResume";
		m_PauseResumeButton.SetLabel(pauseButtonLabel);
		
		// Cancel button
		m_CancelButton.SetEnabled(nTotal > 0);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void OnPauseResumeButton()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (!mgr)
			return;
		
		bool paused = mgr.GetDownloadsPaused();
		mgr.SetDownloadsPaused(!paused);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void OnCancelButton()
	{
		// todo: confirmation dialog
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (!mgr)
			return;
		
		array<ref SCR_WorkshopItemActionDownload> actions = mgr.GetDownloadQueue();
		
		foreach (SCR_WorkshopItemActionDownload action : actions)
		{
			action.Cancel();
		}
	}
};