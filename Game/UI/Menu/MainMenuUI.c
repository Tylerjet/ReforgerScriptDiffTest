class MainMenuUI : ChimeraMenuBase
{
	protected SCR_MenuTileComponent m_FocusedTile;
	protected SCR_AccountWidgetComponent m_AccountComponent;
	protected DialogUI m_BannedDetectionDialog;

	protected ref array<string> m_aBannedItems = {};
	protected static ref array<ref SCR_NewsEntry> m_aNews = {};
	protected static ref array<ref SCR_NewsEntry> m_aNotifications = {};

	protected static const int DEFAULT_XBOX_SERIES_S_PRESET = 5;
	protected static const int DEFAULT_XBOX_SERIES_X_PRESET = 4;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

#ifdef PLATFORM_CONSOLE
		SetupXboxDefaultVideoSettings();
#endif
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

		// Activate account widget
		Widget account = w.FindAnyWidget("AccountWidget");
		if (account)
			m_AccountComponent = SCR_AccountWidgetComponent.Cast(account.FindHandler(SCR_AccountWidgetComponent));

		SCR_ButtonImageComponent logo = SCR_ButtonImageComponent.GetButtonImage("LogoButton", w);
		if (logo)
			logo.m_OnClicked.Insert(OnLogoClicked);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		// Opening Last menu
		SCR_MenuLoadingComponent.LoadLastMenu();
		SCR_MenuLoadingComponent.ClearLastMenu();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetRootWidget().SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! add all input listeners
	override void OnMenuFocusGained()
	{
		GetRootWidget().SetEnabled(true);

		if (m_AccountComponent)
			m_AccountComponent.UpdateNotifications();

		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget(), true);

		// Check playing for the first time, do not show to devs
		if (Game.IsDev())
			return;

		bool firstLoad;
		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (cont)
			cont.Get("m_bFirstTimePlay", firstLoad);
		
		if (!firstLoad)
			return;

		// Complete the first load, show welcome screen
		cont.Set("m_bFirstTimePlay", false);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		if (!IsFocused())
			return;
				
		TryExitGame();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCredits()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsMenu);
	}

	//------------------------------------------------------------------------------------------------
	void OnTileClick(notnull SCR_MenuTileComponent comp)
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
	void OnTileFocus(SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		m_FocusedTile = comp;
	}

	//------------------------------------------------------------------------------------------------
	void OnButtonSelect()
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
	void OnFeedback()
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
	void RestoreFocus()
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
		for(int i, cnt = GetGame().GetBackendApi().GetNotifyCount(); i < cnt; i++)
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
	static void TryExitGame()
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
	
	//---------------------------------------------------------------------------------------------
	//solution to different presets for series X and S, since we don't have different defaults for them in WB so far
	static void SetupXboxDefaultVideoSettings()
	{
		BaseContainer display = GetGame().GetEngineUserSettings().GetModule("DisplayUserSettings");
		UserSettings video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
		BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (!display || !video || !videoSettings)
			return;
		if (System.GetPlatform() == EPlatform.XBOX_SERIES_S)
		{
			display.Set("OverallQuality", DEFAULT_XBOX_SERIES_S_PRESET);
			video.Set("ResolutionScale", 0.6);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			video.Set("Vsynch", true);
		}
		if (System.GetPlatform() == EPlatform.XBOX_SERIES_X)
		{
			display.Set("OverallQuality", DEFAULT_XBOX_SERIES_X_PRESET);
			video.Set("ResolutionScale", 0.9);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.BOKEH);
			video.Set("Vsynch", true);
		}
		GetGame().ApplySettingsPreset();
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
}
