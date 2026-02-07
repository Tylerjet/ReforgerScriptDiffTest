//------------------------------------------------------------------------------------------------
//! Base deploy menu class.
class SCR_DeployMenuBase : ChimeraMenuBase
{
	protected SCR_NavigationButtonComponent m_PauseButton;
	protected SCR_NavigationButtonComponent m_GameMasterButton;
	protected SCR_NavigationButtonComponent m_ChatButton;

	protected SCR_ChatPanel m_ChatPanel;
	protected SCR_MapEntity m_MapEntity;

	protected static ref OnDeployMenuOpenInvoker s_OnMenuOpen;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if (s_OnMenuOpen)
			s_OnMenuOpen.Invoke();

		m_GameMasterButton = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Editor", GetRootWidget());
		if (m_GameMasterButton)
		{
			SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
			if (editor)
			{
				m_GameMasterButton.SetVisible(!editor.IsLimitedInstance());
				editor.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		// Mute sounds
		// If menu is opened before loading screen is closed, wait for closing
		if (ArmaReforgerLoadingAnim.IsOpen())
			ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(MuteSounds);
		else
			MuteSounds();		
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		MuteSounds(false);
		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		MuteSounds(false);
		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenPlayerList()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.PlayerListMenu);
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

	//! If limited, don't show the game master switch button.
	protected void OnEditorLimitedChanged(bool limited)
	{
		m_GameMasterButton.SetVisible(!limited);
	}

	//------------------------------------------------------------------------------------------------
	static OnDeployMenuOpenInvoker SGetOnMenuOpen()
	{
		if (!s_OnMenuOpen)
			s_OnMenuOpen = new OnDeployMenuOpenInvoker();
		
		return s_OnMenuOpen;
	}
}

//------------------------------------------------------------------------------------------------
//! Main deploy menu with the map present.
class SCR_DeployMenuMain : SCR_DeployMenuBase
{
	protected SCR_DeployMenuHandler m_MenuHandler;

	protected SCR_LoadoutRequestUIComponent m_LoadoutRequestUIHandler;
	protected SCR_GroupRequestUIComponent m_GroupRequestUIHandler;
	protected SCR_SpawnPointRequestUIComponent m_SpawnPointRequestUIHandler;

	protected ref MapConfiguration m_MapConfigDeploy = new MapConfiguration();
	protected SCR_MapUIElementContainer m_UIElementContainer;

	protected SCR_BaseGameMode m_GameMode;
	protected SCR_RespawnComponent m_SpawnRequestManager;
	protected RplId m_iSelectedSpawnPointId = RplId.Invalid();

	protected Widget m_wLoadingSpinner;
	protected SCR_LoadingSpinner m_LoadingSpinner;

	protected FactionManager m_FactionManager;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;

	protected SCR_PlayerLoadoutComponent m_PlyLoadoutComp;

	protected Widget m_wRespawnButton;
	protected SCR_DeployButton m_RespawnButton;

	protected Widget m_wMenuFrame;

	protected SCR_RespawnTimerComponent m_ActiveRespawnTimer;	
	protected SCR_RespawnTimerComponent m_PlayerRespawnTimer;
	protected SCR_TimedSpawnPointComponent m_TimedSpawnPointTimer;
	protected int m_iPreviousTime = 0;
	protected bool m_bRespawnRequested = false;
	protected SCR_RespawnSystemComponent m_RespawnSystemComp;

	protected int m_iPlayerId;
	
	protected bool m_bMapContextAllowed = true;
	
	//------------------------------------------------------------------------------------------------
	//! Sets map context active based on whether or not any of the selectors are focused with a gamepad.
	void AllowMapContext(bool allow)
	{
		m_bMapContextAllowed = allow;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_wMenuFrame = GetRootWidget().FindAnyWidget("MenuFrame");

		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_MenuHandler = SCR_DeployMenuHandler.Cast(GetRootWidget().FindHandler(SCR_DeployMenuHandler));
		m_RespawnSystemComp = SCR_RespawnSystemComponent.GetInstance();
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
		{
			Debug.Error("Map entity is missing in the world! Deploy menu won't work correctly.");
		}

		FindRequestHandlers();

		m_wRespawnButton = GetRootWidget().FindAnyWidget("RespawnButton");

		m_RespawnButton = SCR_DeployButton.Cast(m_wRespawnButton.FindHandler(SCR_DeployButton));
		if (m_RespawnButton)
			m_RespawnButton.m_OnClicked.Insert(RequestRespawn);

		Widget spinnerRoot = GetRootWidget().FindAnyWidget("LoadingSpinner");
		m_wLoadingSpinner = spinnerRoot.FindAnyWidget("Spinner");
		if (m_wLoadingSpinner)
			m_LoadingSpinner = SCR_LoadingSpinner.Cast(m_wLoadingSpinner.FindHandler(SCR_LoadingSpinner));

		m_FactionManager = GetGame().GetFactionManager();
		if (!m_FactionManager)
		{
			Print("Cannot find faction manager, respawn menu functionality will be broken.", LogLevel.ERROR);
		}

		m_PlayerRespawnTimer = SCR_RespawnTimerComponent.Cast(m_GameMode.FindComponent(SCR_RespawnTimerComponent));
		m_TimedSpawnPointTimer = SCR_TimedSpawnPointComponent.Cast(m_GameMode.FindComponent(SCR_TimedSpawnPointComponent));
		m_ActiveRespawnTimer = m_PlayerRespawnTimer;

		PlayerController pc = GetGame().GetPlayerController();
		m_iPlayerId = pc.GetPlayerId();

		m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!m_PlyFactionAffilComp)
		{
			Print("Cannot find player faction affiliation component!", LogLevel.ERROR);
		}

		m_PlyLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!m_PlyLoadoutComp)
		{
			Print("Cannot find player loadout component!", LogLevel.ERROR);
		}

		m_SpawnRequestManager = SCR_RespawnComponent.Cast(pc.GetRespawnComponent());

		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_NavigationButtonComponent.GetNavigationButtonComponent("ChatButton", GetRootWidget());
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);

		m_PauseButton = SCR_NavigationButtonComponent.GetNavigationButtonComponent("PauseButton", GetRootWidget());
		if (m_PauseButton)
			m_PauseButton.m_OnActivated.Insert(OnPauseMenu);

		HookEvents();
		InitMapDeploy();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();

		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		GetGame().GetInputManager().RemoveActionListener("InstantVote", EActionTrigger.DOWN, GetGame().OnInstantVote);
		GetGame().GetInputManager().RemoveActionListener("DeployMenuSelect", EActionTrigger.DOWN, RequestRespawn);
		
		GetGame().GetInputManager().RemoveActionListener("SpawnPointNext", EActionTrigger.DOWN, NextSpawn);
		GetGame().GetInputManager().RemoveActionListener("SpawnPointPrev", EActionTrigger.DOWN, PrevSpawn);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{ 
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();

		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		GetGame().GetInputManager().AddActionListener("InstantVote", EActionTrigger.DOWN, GetGame().OnInstantVote);
		GetGame().GetInputManager().AddActionListener("DeployMenuSelect", EActionTrigger.DOWN, RequestRespawn);

		GetGame().GetInputManager().AddActionListener("SpawnPointNext", EActionTrigger.DOWN, NextSpawn);
		GetGame().GetInputManager().AddActionListener("SpawnPointPrev", EActionTrigger.DOWN, PrevSpawn);
	}

	//! Select next available spawn point.
	protected void NextSpawn()
	{
		if (!m_bRespawnRequested)
			m_SpawnPointRequestUIHandler.CycleSpawnPoints();
	}

	//! Select previous available spawn point.
	protected void PrevSpawn()
	{
		if (!m_bRespawnRequested)
			m_SpawnPointRequestUIHandler.CycleSpawnPoints(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		Faction plyFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();

		m_LoadoutRequestUIHandler.ShowAvailableLoadouts(plyFaction);
		m_GroupRequestUIHandler.ShowAvailableGroups(plyFaction);
		m_SpawnPointRequestUIHandler.ShowAvailableSpawnPoints(plyFaction);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		GetGame().GetInputManager().ActivateContext("DeployMenuContext");
		GetGame().GetInputManager().ActivateContext("DeployMenuMapContext");
		
		if (m_bMapContextAllowed && m_MapEntity && m_MapEntity.IsOpen())
			GetGame().GetInputManager().ActivateContext("MapContext");
		
		if (m_LoadingSpinner && m_wLoadingSpinner.IsVisible())
		{
			m_LoadingSpinner.Update(tDelta);			
			m_RespawnButton.UpdateSpinner(tDelta);
		}

		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);

		if (m_ActiveRespawnTimer)
		{
			int remainingTime = m_ActiveRespawnTimer.GetPlayerRemainingTime(m_iPlayerId);
			if (remainingTime > 0)
			{
				if (remainingTime != m_iPreviousTime)
				{
					SCR_UISoundEntity.SetSignalValueStr("countdownValue", remainingTime);
					SCR_UISoundEntity.SetSignalValueStr("maxCountdownValue", m_ActiveRespawnTimer.GetRespawnTime());					
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN);
					m_iPreviousTime = remainingTime;
				}

				string respawnTime = SCR_Global.GetTimeFormatting(
					remainingTime,
					ETimeFormatParam.DAYS | ETimeFormatParam.HOURS,
					ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);				
				m_RespawnButton.SetText(respawnTime);
				m_RespawnButton.SetEnabled(false);
			}
			else
			{
				if (m_iPreviousTime == 1)
				{
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN_END);
					m_iPreviousTime = remainingTime;
				}
				
				m_RespawnButton.SetText();
				UpdateRespawnButton();
			}
		}
	}

	//! Find all components in the layout that are needed in order for deploy menu to function correctly.
	protected void FindRequestHandlers()
	{
		m_LoadoutRequestUIHandler = m_MenuHandler.GetLoadoutRequestHandler();
		if (!m_LoadoutRequestUIHandler)
		{
			Print("Failed to find SCR_LoadoutRequestUIComponent!", LogLevel.ERROR);
		}

		m_GroupRequestUIHandler = m_MenuHandler.GetGroupRequestHandler();
		if (!m_GroupRequestUIHandler)
		{
			Print("Failed to find SCR_GroupRequestUIComponent!", LogLevel.ERROR);
		}

		m_SpawnPointRequestUIHandler = m_MenuHandler.GetSpawnPointRequestHandler();
		if (!m_SpawnPointRequestUIHandler)
		{
			Print("Failed to find SCR_SpawnPointRequestUIComponent!", LogLevel.ERROR);
		}
	}

	//! Initialize necessary callbacks.
	protected void HookEvents()
	{
		m_PlyFactionAffilComp.GetOnPlayerFactionRequestInvoker_O().Insert(OnPlayerFactionRequest);
		m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		m_PlyLoadoutComp.GetOnPlayerLoadoutRequestInvoker_O().Insert(OnPlayerLoadoutRequest);
		m_PlyLoadoutComp.GetOnPlayerLoadoutResponseInvoker_O().Insert(OnPlayerLoadoutResponse);

		m_SpawnRequestManager.GetOnRespawnRequestInvoker_O().Insert(OnRespawnRequest);
		m_SpawnRequestManager.GetOnRespawnResponseInvoker_O().Insert(OnRespawnResponse);
		
		m_SpawnPointRequestUIHandler.GetOnSpawnPointSelected().Insert(SetSpawnPoint);
		m_GroupRequestUIHandler.GetOnPlayerGroupJoined().Insert(OnPlayerGroupJoined);
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);

		m_GameMode.GetOnPreloadFinished().Insert(HideLoading);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen()
	{
		m_MapEntity.SetZoom(1);

		// note@lk: temporary hotfix for duplicite journal entries, better solution is on the way
		Widget toolMenu = m_wMenuFrame.FindAnyWidget("ToolMenuVert");
		Widget child = toolMenu.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}		
		
		m_UIElementContainer = SCR_MapUIElementContainer.Cast(m_MapEntity.GetMapUIComponent(SCR_MapUIElementContainer));
		if (m_UIElementContainer)
			m_UIElementContainer.GetOnSpawnPointSelected().Insert(SetSpawnPointExt);
		
		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId);
		if (sp)
			GetGame().GetCallqueue().CallLater(SetInitialSpawnPoint, 0, false, m_iSelectedSpawnPointId); // called the next frame because of widget init order
	}

	//! Callback when player joins a group
	protected void OnPlayerGroupJoined(SCR_AIGroup group)
	{
	}

	//! Initializes the map with deploy menu config.
	protected void InitMapDeploy()
	{
		if (!m_MapEntity || m_MapEntity.IsOpen())
			return;

		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(m_GameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;

		m_MapConfigDeploy = m_MapEntity.SetupMapConfig(EMapEntityMode.SPAWNSCREEN, configComp.GetSpawnMapConfig(), GetRootWidget());
		m_MapEntity.OpenMap(m_MapConfigDeploy);
	}

	//! Sets spawn point from an external source (ie. by clicking the spawn point icon).
	protected void SetSpawnPointExt(RplId id)
	{
		m_SpawnPointRequestUIHandler.SelectSpawnPointExt(id);
	}

	//! Sets initial spawn point when the deploy map is open for the first time.
	protected void SetInitialSpawnPoint(RplId spawnPointId)
	{
		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId);
		SetSpawnPoint(spawnPointId, false);
	}

	//! Sets the currently selected spawn point.
	protected void SetSpawnPoint(RplId id, bool smoothPan = true)
	{
		m_iSelectedSpawnPointId = id;

		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(id);
		if (!sp)
			return;

		FocusOnPoint(sp, smoothPan);

		if (m_UIElementContainer)
			m_UIElementContainer.OnSpawnPointSelectedExt(id);
		
		if (m_TimedSpawnPointTimer)
		{
			if (sp.IsTimed())
				m_ActiveRespawnTimer = m_TimedSpawnPointTimer;
			else
				m_ActiveRespawnTimer = m_PlayerRespawnTimer;
		}
	}

	//! Centers map to a specific spawn point.
	protected void FocusOnPoint(notnull SCR_SpawnPoint spawnPoint, bool smooth = true)
	{
		if (!m_MapEntity || !m_MapEntity.IsOpen())
			return;

		vector o = spawnPoint.GetOrigin();

		float x, y;
		m_MapEntity.WorldToScreen(o[0], o[2], x, y);

		float xScaled = GetGame().GetWorkspace().DPIUnscale(x);
		float yScaled = GetGame().GetWorkspace().DPIUnscale(y);

		if (smooth)
			m_MapEntity.PanSmooth(x, y);
		else
			m_MapEntity.SetPan(xScaled, yScaled);
	}

	//! Hides loading spinner widget.
	protected void HideLoading()
	{
		m_wLoadingSpinner.SetVisible(false);
	}

	//! Sends a respawn request based on assigned loadout and selected spawn point.
	protected void RequestRespawn()
	{
		if (!m_RespawnButton.IsEnabled())
			return;

		if (!m_iSelectedSpawnPointId.IsValid())
		{
			Debug.Error("Selected SpawnPointId is invalid!");
			return;
		}

		ResourceName resourcePrefab = ResourceName.Empty;
		if (m_LoadoutRequestUIHandler.GetPlayerLoadout())
			resourcePrefab = m_LoadoutRequestUIHandler.GetPlayerLoadout().GetLoadoutResource();
		else
		{
			Debug.Error("No player loadout assigned!");
			return;
		}

		SCR_SpawnPointSpawnData rspData = new SCR_SpawnPointSpawnData(resourcePrefab, m_iSelectedSpawnPointId);
		m_SpawnRequestManager.RequestSpawn(rspData);
	}

	//! Callback when player requests a faction.
	protected void OnPlayerFactionRequest(SCR_PlayerFactionAffiliationComponent component, int factionIndex)
	{
		m_wLoadingSpinner.SetVisible(true);
	}

	//! Callback when faction request receives a response.
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			Faction assignedFaction = m_FactionManager.GetFactionByIndex(factionIndex);
			OnPlayerFactionSet(assignedFaction);
		}

		m_wLoadingSpinner.SetVisible(false);
	}

	//! Callback when player requests a loadout.
	protected void OnPlayerLoadoutRequest(SCR_PlayerLoadoutComponent component, int loadoutIndex)
	{
		m_wLoadingSpinner.SetVisible(true);
	}

	//! Callback when loadout request receives a response.
	protected void OnPlayerLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		if (response)
		{
			m_LoadoutRequestUIHandler.RefreshLoadoutPreview();
			m_RespawnButton.SetEnabled(true);
		}

		m_wLoadingSpinner.SetVisible(false);
		if (m_LoadoutRequestUIHandler)
			m_LoadoutRequestUIHandler.Unlock();
	}

	//----------------------------------------------------------------------------------------------
	protected void OnPlayerFactionSet(Faction assignedFaction)
	{
		if (m_LoadoutRequestUIHandler)
			m_LoadoutRequestUIHandler.ShowAvailableLoadouts(assignedFaction);
		
		if (m_GroupRequestUIHandler)
			m_GroupRequestUIHandler.ShowAvailableGroups(assignedFaction);
		
		if (m_SpawnPointRequestUIHandler)
			m_SpawnPointRequestUIHandler.ShowAvailableSpawnPoints(assignedFaction);

	}

	//! Toggle chat.
	protected void OnChatToggle()
	{
		if (!m_ChatPanel)
			return;

		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}

	//! Opens pause menu.
	protected void OnPauseMenu()
	{
		GetGame().OpenPauseMenu(false, true);
	}

	//! Callback when respawn request was sent for the player.
	protected void OnRespawnRequest(SCR_SpawnRequestComponent requestComponent)
	{
		m_bRespawnRequested = true;		
		GetRootWidget().SetEnabled(false);
		m_RespawnButton.SetEnabled(false);
		m_RespawnButton.ShowLoading(false);
		m_wLoadingSpinner.SetVisible(true);

		UpdateRespawnButton();
	}

	//! Callback when player respawn request received a response.
	protected void OnRespawnResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		GetRootWidget().SetEnabled(true);
		m_wLoadingSpinner.SetVisible(false);
		if (response != SCR_ESpawnResult.OK)
		{
			m_RespawnButton.SetEnabled(true);
			m_RespawnButton.ShowLoading(true);
		}
		m_bRespawnRequested = false;
	}

	//! Sets respawn button enabled based on certain conditions.
	protected void UpdateRespawnButton()
	{
		m_RespawnButton.SetEnabled(!m_bRespawnRequested && m_RespawnSystemComp.IsRespawnEnabled() && m_PlyLoadoutComp.GetLoadout() != null);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens deploy menu.
	static SCR_DeployMenuMain OpenDeployMenu()
	{
		if (!GetDeployMenu())
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.RespawnSuperMenu);
		
		return GetDeployMenu();
	}

	//! As the name suggests, this method closes the deploy menu instance.
	static void CloseDeployMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
	}

	//! Returns the deploy menu instance.
	static SCR_DeployMenuMain GetDeployMenu()
	{
		return SCR_DeployMenuMain.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu));
	}
};

//------------------------------------------------------------------------------------------------
//! Component that handles the request respawn button.
class SCR_DeployButton : SCR_ButtonBaseComponent
{
	[Attribute("Text")]
	protected string m_sText;
	protected TextWidget m_wText;

	[Attribute("Spinner")]
	protected string m_sLoadingSpinner;
	protected Widget m_wLoadingSpinner;

	protected SCR_LoadingSpinner m_LoadingSpinner;
	
	protected string m_sTextDefault = "<action name=\"DeployMenuSelect\"/>#AR-ButtonSelectDeploy";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sText));
		m_wLoadingSpinner = w.FindAnyWidget(m_sLoadingSpinner);
		m_LoadingSpinner = SCR_LoadingSpinner.Cast(m_wLoadingSpinner.FindHandler(SCR_LoadingSpinner));
	}

	//! Set text of the button.
	void SetText(string text = string.Empty)
	{
		if (text.IsEmpty())
			text = m_sTextDefault;
		else
			text = string.Format("%1 %2", "#AR-DeployMenu_DeployIn", text);

		if (m_wText)
			m_wText.SetTextFormat(text);
	}

	//! Update the loading spinner widget.
	void UpdateSpinner(float timeSlice)
	{
		if (m_wLoadingSpinner && m_wLoadingSpinner.IsVisible())
			m_LoadingSpinner.Update(timeSlice);
	}

	//! Set loading spinner widget visible.
	void ShowLoading(bool show)
	{
		m_wText.SetVisible(show);
		m_wLoadingSpinner.SetVisible(!show);
	}
};