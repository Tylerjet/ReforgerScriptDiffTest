enum EDeployScreenType
{
	BRIEFING = 0,
	FACTION = 1,
	GROUP = 2,
	LOADOUT = 3,
	MAP = 4
};

//------------------------------------------------------------------------------------------------
class SCR_RespawnSuperMenu : SCR_SuperMenuBase
{
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	protected SCR_RespawnMenuHandlerComponent m_RespawnMenuHandler;
	protected SCR_LoadingOverlay m_Loading;

	protected static SCR_RespawnSuperMenu s_Instance;
	protected static bool s_bIsShown;
	protected bool m_bAllowDeploy = true;
	protected bool m_bQuickDeployToggled;

	static ref ScriptInvoker Event_OnMenuOpen = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuClose = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuShow = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuHide = new ScriptInvoker();

	protected string m_sBack = "Back";
	protected string m_sTitle = "Title";
	
	protected SCR_ChatPanel m_ChatPanel;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		Event_OnMenuOpen.Invoke();

		Widget backBtn = GetRootWidget().FindAnyWidget(m_sBack);
		if (backBtn)
		{
			SCR_NavigationButtonComponent nav = SCR_NavigationButtonComponent.Cast(backBtn.FindHandler(SCR_NavigationButtonComponent));
			nav.m_OnActivated.Insert(OnMenuBack);
		}

		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!m_RespawnSystemComponent)
		{
			Print("Respawn menu could not find any RespawnSystemComponent! Is one attached to the GameMode?", LogLevel.ERROR);
			return;
		}

		m_RespawnMenuHandler = SCR_RespawnMenuHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_RespawnMenuHandlerComponent));
		if (!m_RespawnMenuHandler)
		{
			Print("Can't find SCR_RespawnMenuHandlerComponent! This should never happen!", LogLevel.ERROR);
			return;
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
		{
			Print("Respawn menu could not find any SCR_BaseGameMode", LogLevel.ERROR);
			return;
		}

		Widget loading = GetRootWidget().FindAnyWidget("Loading");
		if (loading)
		{
			m_Loading = SCR_LoadingOverlay.Cast(loading.FindHandler(SCR_LoadingOverlay));
		}

		super.OnMenuOpen();
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.PauseMenu);

		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return;

		array<Faction> factions = {};
		int count = fm.GetFactionsList(factions);
		for (int i = 0; i < count; ++i)
		{
			SCR_Faction faction = SCR_Faction.Cast(factions[i]);
			faction.GetOnFactionPlayableChanged().Insert(UpdateRespawnMenu);
		}
		
		UpdateTabs();
		
		SCR_NavigationButtonComponent chat = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Chat", GetRootWidget());
		if (chat)
			chat.m_OnActivated.Insert(OnChatToggle);
		
		// Find chat panel
		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		// Mute sounds
		// If menu is opened before loading screen is closed, wait for closing
		if (ArmaReforgerLoadingAnim.IsOpen())
			ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(MuteSounds);
		else
			MuteSounds();
		
		GetGame().GetCallqueue().CallLater(m_RespawnMenuHandler.DestroyPreloadPlaceholder, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRespawnMenu(Faction faction, bool available)
	{
		int pid = SCR_PlayerController.GetLocalPlayerId();
		Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(pid);
		if (playerFaction == faction && !available)
		{
			SCR_RespawnComponent rc = SCR_RespawnComponent.GetInstance();
			if (rc)
			{
				rc.RequestClearPlayerFaction();
				rc.RequestClearPlayerLoadout();
				rc.RequestClearPlayerSpawnPoint();
			}
		}

		UpdateCurrentTab();
	}
	
	//------------------------------------------------------------------------------------------------
	void MuteSounds(bool mute = true)
	{
		if (!IsOpen())
			return;
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, !mute);
		
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Remove(MuteSounds);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnChatToggle()
	{
		if (!m_ChatPanel)
			return;
		
		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuBack()
	{
		if (s_bIsShown)
			GetGame().OpenPauseMenu(false, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		MuteSounds(false);
		GetRootWidget().SetEnabled(true);
		Event_OnMenuClose.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (!s_bIsShown)
			Event_OnMenuShow.Invoke();
		s_bIsShown = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		MuteSounds(false);
		s_bIsShown = false;
		Event_OnMenuHide.Invoke();
		//GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		GetRootWidget().SetEnabled(true);
		// Auto-open the editor (when enabled) on top of respawn menu for Game Masters
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();
		
		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
		GetGame().GetInputManager().AddActionListener("InstantVote", EActionTrigger.DOWN, GetGame().OnInstantVote);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		GetRootWidget().SetEnabled(false);
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
		GetGame().GetInputManager().RemoveActionListener("InstantVote", EActionTrigger.DOWN, GetGame().OnInstantVote);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowPlayerList()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PlayerListMenu, 0, true, false);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateTabs()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		int selectedTab = m_TabViewComponent.GetShownTab();
		SCR_RespawnBriefingComponent briefingComponent = SCR_RespawnBriefingComponent.GetInstance();
		SCR_Faction playerFaction = SCR_Faction.Cast(m_RespawnSystemComponent.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId()));
		bool isFactionAssigned = (
			playerFaction != null
			&& playerFaction.IsPlayable()
		);
		bool isLoadoutAssigned = (m_RespawnSystemComponent.GetPlayerLoadout(SCR_PlayerController.GetLocalPlayerId()) != null);
		bool isSpawnPointAssigned = (m_RespawnSystemComponent.GetPlayerSpawnPoint(SCR_PlayerController.GetLocalPlayerId()) != null);
		bool isGroupConfirmed = true;
		if (groupsManager)
			isGroupConfirmed = groupsManager.GetConfirmedByPlayer();
		
		// enable individual submenu tabs based on gamemode settings:
		m_TabViewComponent.SetTabVisible(EDeployScreenType.BRIEFING, briefingComponent && briefingComponent.GetInfo()); 

	 	m_TabViewComponent.EnableTab(EDeployScreenType.FACTION,
	 		(m_RespawnMenuHandler.GetAllowFactionSelection()
	 		&& (m_RespawnMenuHandler.GetAllowFactionChange() || !isFactionAssigned))
	 	);

		m_TabViewComponent.EnableTab(EDeployScreenType.GROUP, isFactionAssigned && groupsManager);

		m_TabViewComponent.EnableTab(EDeployScreenType.LOADOUT,
			(isFactionAssigned
			&& m_RespawnMenuHandler.GetAllowLoadoutSelection()
			&& isGroupConfirmed)
		);

		bool showMap = (
			m_RespawnMenuHandler.GetAllowSpawnPointSelection()
			&& isFactionAssigned
			&& (isLoadoutAssigned || !m_RespawnMenuHandler.GetAllowLoadoutSelection())
			&& isGroupConfirmed
		);

		m_TabViewComponent.EnableTab(EDeployScreenType.MAP, showMap);

		int nextTab = m_TabViewComponent.GetNextValidItem(false);
		if (m_TabViewComponent.IsTabEnabled(nextTab))
			selectedTab = nextTab;

		// switch to the first enabled tab
		if (showMap)
			selectedTab = EDeployScreenType.MAP;
		else if (!m_TabViewComponent.IsTabEnabled(selectedTab))
			selectedTab = m_TabViewComponent.GetNextValidItem(false);

		if (selectedTab > -1) // todo(hajekmar): should this be handled in ShowTab() instead?
			m_TabViewComponent.ShowTab(selectedTab, true, false);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCurrentTab()
	{
		m_TabViewComponent.ShowTab(m_TabViewComponent.GetShownTab());
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnFactionAssigned(int playerId, Faction faction)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (!faction)
				m_TabViewComponent.ShowTab(EDeployScreenType.FACTION);
			UpdateTabs();
		}

		SCR_SelectFactionSubMenu menu = SCR_SelectFactionSubMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnFactionAssigned(playerId, faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnLoadoutAssigned(int playerId, SCR_BasePlayerLoadout loadout)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (!loadout)
				m_TabViewComponent.ShowTab(EDeployScreenType.FACTION);
			UpdateTabs();
		}

		SCR_SelectLoadoutSubMenu menu = SCR_SelectLoadoutSubMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnLoadoutAssigned(playerId, loadout);
		}
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnSpawnPointAssigned(int playerId, SCR_SpawnPoint spawnPoint)
	{
		SCR_SelectSpawnPointSubMenu menu = SCR_SelectSpawnPointSubMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnSpawnPointAssigned(playerId, spawnPoint);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LeaveServer()
	{
		GameStateTransitions.RequestGameplayEndTransition();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsRespawnMenuOpen()
	{
		return s_bIsShown;
	}

	//------------------------------------------------------------------------------------------------
	void SetMenuTitle(string text)
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sTitle));
		if (w)
			w.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetLoadingVisible(bool visible)
	{
		m_Loading.SetShown(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetQuickDeployToggled()
	{
		return m_bQuickDeployToggled;
	}

	//------------------------------------------------------------------------------------------------
	void ToggleQuickDeploy(bool toggled)
	{
		m_bQuickDeployToggled = toggled;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RespawnMenuHandlerComponent GetRespawnMenuHandler()
	{
		return m_RespawnMenuHandler;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_RespawnSuperMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RespawnSuperMenu()
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RespawnSuperMenu()
	{
		s_Instance = null;
	}
};
