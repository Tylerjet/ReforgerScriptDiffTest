class MainMenuUI : ChimeraMenuBase
{
	protected const string EXPERIMENTAL_LABEL = "#AR-Experimental_WelcomeLabel";
	protected const string EXPERIMENTAL_MESSAGE = "#AR-Experimental_WelcomeMessage";
	
	protected SCR_MenuTileComponent m_FocusedTile;
	protected SCR_AccountWidgetComponent m_AccountComponent;
	protected DialogUI m_BannedDetectionDialog;
	protected DialogUI m_ExperimentalDialog;

	protected ref array<string> m_aBannedItems = {};
	protected static ref array<ref SCR_NewsEntry> m_aNews = {};
	protected static ref array<ref SCR_NewsEntry> m_aNotifications = {};

	protected static bool s_bDidCheckNetwork;
	protected static const int SERVICES_STATUS_CHECK_DELAY = 2000; // needed for backend API to prepare



	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Init common Workshop UI
		SCR_WorkshopUiCommon.OnGameStart();

		Widget w = GetRootWidget();
		Widget content = w.FindAnyWidget("Grid");
		Widget descriptionRoot = w.FindAnyWidget("Description");
		Widget footer = w.FindAnyWidget("Footer");
		if (!content || !descriptionRoot || !footer)
			return;

		// Listen to buttons in grid
		Widget child = content.GetChildren();
		while (child)
		{
			SCR_MenuTileComponent tile = SCR_MenuTileComponent.Cast(child.FindHandler(SCR_MenuTileComponent));
			if (!tile)
				break;

			if (!m_FocusedTile)
				m_FocusedTile = tile;

			tile.m_OnClicked.Insert(OnTileClick);
			tile.m_OnFocused.Insert(OnTileFocus);
			child = child.GetSibling();
		}

		// Subscribe to buttons
		SCR_NavigationButtonComponent back = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", footer);
		if (back)
			back.m_OnActivated.Insert(OnBack);

		// Services Status button
		SCR_NavigationButtonComponent servicesStatus = SCR_NavigationButtonComponent.GetNavigationButtonComponent("ServicesStatus", footer);
		if (servicesStatus)
			servicesStatus.m_OnActivated.Insert(OnServicesStatus);

		SCR_NavigationButtonComponent feedback = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Feedback", footer);
		if (feedback)
			feedback.m_OnActivated.Insert(OnFeedback);

		SCR_NavigationButtonComponent credits = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Credits", footer);
		if (credits)
			credits.m_OnActivated.Insert(OnCredits);

#ifdef WORKBENCH
		GetGame().GetInputManager().AddActionListener("MenuBackWB", EActionTrigger.DOWN, OnBack);
#endif

		// Get the entries for the first time
		GetNewsEntries(null);
		GetNotificationEntries(null);

		// Check Service Statuses
		if (!s_bDidCheckNetwork)
		{
			s_bDidCheckNetwork = true;
			GetGame().GetCallqueue().CallLater(CheckServicesStatusIfFocused, SERVICES_STATUS_CHECK_DELAY);
		}

		// Activate account widget
		Widget account = w.FindAnyWidget("AccountWidget");
		if (account)
			m_AccountComponent = SCR_AccountWidgetComponent.Cast(account.FindHandler(SCR_AccountWidgetComponent));

		SCR_ButtonImageComponent logo = SCR_ButtonImageComponent.GetButtonImage("LogoButton", w);
		if (logo)
			logo.m_OnClicked.Insert(OnLogoClicked);
		
		// Check ping sites 
		GetGame().GetBackendApi().GetClientLobby().MeasureLatency(null);
	}
	
	protected bool m_bFirstLoad;

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpened()
	{
		// Opening Last menu
		SCR_MenuLoadingComponent.LoadLastMenu();
		//SCR_MenuLoadingComponent.ClearLastMenu();

		// Show experimental dialog 
		if (GetGame().IsExperimentalBuild())
		{
			m_ExperimentalDialog = DialogUI.Cast(
				GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ExperimentalDialog, DialogPriority.CRITICAL));
			m_ExperimentalDialog.m_OnConfirm.Insert(OnExperitementalDialogClose);
		}
		
		// Check playing for the first time, do not show to devs
		if (Game.IsDev())
			return;
		
		//bool firstLoad;
		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (cont)
			cont.Get("m_bFirstTimePlay", m_bFirstLoad);
		
		if (!m_bFirstLoad)
			return;
		
		// Complete the first load, show welcome screen, save into settings
		cont.Set("m_bFirstTimePlay", false);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		
		// Prevent opening of welcome dialog - should be displayed on closing experimental dialog 
		if (!GetGame().IsExperimentalBuild())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnExperitementalDialogClose()
	{
		m_ExperimentalDialog.m_OnConfirm.Remove(OnExperitementalDialogClose);	
		
		if (m_bFirstLoad)
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusLost()
	{
		GetRootWidget().SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! add all input listeners
	protected override void OnMenuFocusGained()
	{
		GetRootWidget().SetEnabled(true);

		if (m_AccountComponent)
			m_AccountComponent.UpdateNotifications();

		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget(), true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		if (!IsFocused())
			return;

		TryExitGame();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCredits()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileClick(notnull SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		ChimeraMenuPreset preset = comp.m_eMenuPreset;
		if (preset == ChimeraMenuPreset.FeedbackDialog)
			GetGame().GetMenuManager().OpenDialog(preset);
		else if (preset == ChimeraMenuPreset.ContentBrowser)
			SCR_WorkshopUiCommon.TryOpenWorkshop();
		else
			GetGame().GetMenuManager().OpenMenu(preset);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocus(SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		m_FocusedTile = comp;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonSelect()
	{
		if (!IsFocused())
			return;

		if (m_FocusedTile)
			OnTileClick(m_FocusedTile);
	}

	//------------------------------------------------------------------------------------------------
	void OnServicesStatus()
	{
		if (!IsFocused())
			return;

		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServicesStatusDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFeedback()
	{
		if (!IsFocused())
			return;

		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.FeedbackDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void RestoreFocus()
	{
		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	static int GetNewsEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNews.Clear();

		// Print( "NEWS: Count - " + GetGame().GetBackendApi().GetNewsCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNewsCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNewsItem(i);
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}

		if (entries)
			entries = m_aNews;

		return m_aNews.Count();
	}

	//------------------------------------------------------------------------------------------------
	static int GetNotificationEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNotifications.Clear();

		//Print( "NOTIFICATIONS: Count - " + GetGame().GetBackendApi().GetNotifyCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNotifyCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNotifyItem(i);
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}

		if (entries)
			entries = m_aNotifications;

		return m_aNotifications.Count();
	}

	//------------------------------------------------------------------------------------------------
	static int GetUnreadNewsCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNews)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	static int GetUnreadNotificationCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNotifications)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//---------------------------------------------------------------------------------------------
	//! Shows exit game dialog, or some other dialog if something should
	//! prevent user from exiting the game through the main menu
	protected static void TryExitGame()
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal > 0)
			new SCR_ExitGameWhileDownloadingDialog();
		else
			new SCR_ExitGameDialog();
	}

	//------------------------------------------------------------------------------------------------
	//! Check Services Status only if the main menu is still focused (given the check is delayed by SERVICES_STATUS_CHECK_DELAY)
	protected void CheckServicesStatusIfFocused()
	{
		if (!IsFocused())
			return;

		SCR_ServicesStatusDialogUI.OpenIfServicesAreNotOK();
	}
};
