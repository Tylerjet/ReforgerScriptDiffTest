/*
Main dialog of the download manager, which is typically opened from the download manager panel.

It lists all current and previous downloads.

!!! For the menu to work, it needs a SCR_DownloadManager_Entity to be placed in the world.
This is required because the menu can be opened after any of the downloads have been started,
but by that time we must store the download references somewhere.
*/
class SCR_DownloadManager_Dialog : DialogUI
{
	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{6A8980469B6C2285}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineInteractive.layout";
	
	protected ref SCR_DownloadManager_MiniDialogWidgets m_Widgets = new SCR_DownloadManager_MiniDialogWidgets();
	
	
	//-----------------------------------------------------------------------------------------------
	static SCR_DownloadManager_Dialog Create()
	{
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
	
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		this.InitWidgets();
		
		this.InitList();
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		m_Widgets.Init(GetRootWidget());
		
		m_Widgets.m_PauseButtonComponent.m_OnActivated.Insert(OnPauseButton);
		m_Widgets.m_CancelButtonComponent.m_OnActivated.Insert(OnCancelButton);
		m_Widgets.m_PauseAllButtonComponent.m_OnActivated.Insert(OnPauseAllButton);
		m_Widgets.m_BackButtonComponent.m_OnActivated.Insert(OnBackButton);
		
		// Init buttons from settings
		SCR_WorkshopSettings settings = SCR_WorkshopSettings.Get();
		m_Widgets.m_AutoenableAddonsButtonComponent.SetToggled(settings.m_bAutoEnableDownloadedAddons);
		m_Widgets.m_AutoenableAddonsButtonComponent.m_OnToggled.Insert(OnAutoenableButton);
		
		GetGame().GetInputManager().AddActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
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
		m_Widgets.m_PauseAllButtonComponent.SetLabel(pauseAllButtonLabel);
		m_Widgets.m_PauseAllButtonComponent.SetEnabled(nTotal > 0); // Can't pause all downloads when there are no downloads
		
		// Cancel button
		m_Widgets.m_CancelButtonComponent.SetEnabled(pauseCancelEnabled);
		
		// Pause button
		string pauseButtonLabel = "#AR-DownloadManager_ButtonPause";
		if (dlAction)
		{
			if (dlAction.IsPaused() || dlAction.IsInactive())
				pauseButtonLabel = "#AR-DownloadManager_ButtonResume";
		}
		m_Widgets.m_PauseButtonComponent.SetLabel(pauseButtonLabel);
		m_Widgets.m_PauseButtonComponent.SetEnabled(pauseCancelEnabled);
		
		// Show text when empty
		m_Widgets.m_NoItemsText.SetVisible(m_Widgets.m_DownloadsList.GetChildren() == null);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	// Init list of downloads
	protected void InitList()
	{
		array<ref SCR_DownloadManager_Entry> downloads = new array<ref SCR_DownloadManager_Entry>;
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		foreach (SCR_DownloadManager_Entry dl : downloads)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, m_Widgets.m_DownloadsList);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForDownloadAction(dl.m_Action);
		}
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected SCR_WorkshopItemActionDownload GetSelectedAction()
	{
		Widget w = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (!w)
			return null;
		
		SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
		
		if (!comp)
			return null;
		
		return comp.GetDownloadAction();
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void OnBackButton()
	{
		this.Close();
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
		settings.m_bAutoEnableDownloadedAddons = m_Widgets.m_AutoenableAddonsButtonComponent.GetToggled();
		SCR_WorkshopSettings.Save(settings);
	}
};

/*!
A mini download manager dialog. It only has a few actions and a basic download progress indicator.
*/

class SCR_DownloadManager_MiniDialog : SCR_ConfigurableDialogUi
{
	SCR_NavigationButtonComponent m_PauseResumeButton;
	SCR_NavigationButtonComponent m_CancelButton;
	
	//-----------------------------------------------------------
	static SCR_DownloadManager_MiniDialog Create()
	{
		SCR_DownloadManager_MiniDialog dlg = new SCR_DownloadManager_MiniDialog();
		SCR_DownloadManager_MiniDialog.CreateFromPreset(SCR_WorkshopUiCommon.DIALOGS_CONFIG, "download_manager_mini", dlg);
		return dlg;
	}
	
	
	
	//-----------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		m_PauseResumeButton = this.FindButton("download_pause_resume");
		m_CancelButton = this.FindButton("download_cancel");
		
		m_PauseResumeButton.m_OnActivated.Insert(OnPauseResumeButton);
		m_CancelButton.m_OnActivated.Insert(OnCancelButton);
	}
	
	
	
	//-----------------------------------------------------------
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
	
	
	
	//-----------------------------------------------------------
	protected void OnPauseResumeButton()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (!mgr)
			return;
		
		bool paused = mgr.GetDownloadsPaused();
		mgr.SetDownloadsPaused(!paused);
	}
	
	
	
	//-----------------------------------------------------------
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