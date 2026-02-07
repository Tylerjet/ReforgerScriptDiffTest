//------------------------------------------------------------------------------------------------
class SCR_RespawnSubMenuBase : SCR_SubMenuBase
{
	protected SCR_DeployMenuTileSelection m_TileSelection;

	protected InputManager m_InputManager;
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	protected FactionManager m_FactionManager;
	protected SCR_BaseGameMode m_GameMode;
	protected int m_iPlayerId;
	protected Faction m_SelectedFaction;

	protected SCR_NavigationButtonComponent m_ConfirmButton;
	protected SCR_NavigationButtonComponent m_QuickDeployButton;
	protected string m_sConfirmButtonText;

	protected bool m_bCountdownHidden;
	protected bool m_bTimedSpawnPoint;
	protected bool m_bIsLastAvailableTab;
	protected bool m_bQuickDeployAvailable;
	protected bool m_bButtonsUnlocked = true;
	protected bool m_bDeployRequestSent = false;
	protected bool m_bConfirmButtonEnabled = true;
	protected static bool m_bSpawnPointsAvailable;
	protected static bool s_bPlayableFactionsAvailable;

	protected SCR_RespawnTimerComponent m_Timer;
	protected SCR_TimedSpawnPointComponent m_SpawnPointTimer;
	protected int m_iLastTime;

	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset")]
	protected ResourceName m_sIcons;

	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sButtonTextSelectFaction;

	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sButtonTextSelectLoadout;

	[Attribute("#AR-ButtonSelectDeploy")]
	protected LocalizedString m_sButtonTextSelectDeploy;

	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sConfirm;

	[Attribute("#AR-DeployMenu_QuickDeploy")]
	protected string m_sQuickDeploy;

	//------------------------------------------------------------------------------------------------
	protected void GetWidgets()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		GetWidgets();
		m_bDeployRequestSent = false;

		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;

		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!m_RespawnSystemComponent)
			return;

		m_FactionManager = GetGame().GetFactionManager();
		if (!m_FactionManager)
			return;

		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
			return;

		m_Timer = SCR_RespawnTimerComponent.Cast(m_GameMode.FindComponent(SCR_RespawnTimerComponent));
		m_SpawnPointTimer = SCR_TimedSpawnPointComponent.Cast(m_GameMode.FindComponent(SCR_TimedSpawnPointComponent));
		m_iPlayerId = SCR_PlayerController.GetLocalPlayerId();

		SetDeployAvailable();
		SetQuickDeployAvailable();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		GetGame().GetWorkspace().SetFocusedWidget(null);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		if (m_bIsLastAvailableTab)
			m_sConfirmButtonText = m_sButtonTextSelectDeploy;

		m_ConfirmButton.SetLabel(m_sConfirmButtonText);
		if (m_QuickDeployButton)
		{
			SCR_RespawnSuperMenu rm = SCR_RespawnSuperMenu.Cast(m_ParentMenu);
			m_QuickDeployButton.SetToggled(rm.GetQuickDeployToggled(), false);
		}
		SetQuickDeployAvailable();
	}

	int GetPlayerRemainingRespawnTime(int pid)
	{
		if (!m_Timer)
			return 0;

		int time = m_GameMode.GetPlayerRemainingRespawnTime(pid);
		if (m_bTimedSpawnPoint && m_SpawnPointTimer)
			time = m_SpawnPointTimer.GetPlayerRemainingTime(pid);

		return time;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		if (m_InputManager)
			m_InputManager.ActivateContext("DeployMenuContext");

		if (m_GameMode && m_Timer && m_RespawnSystemComponent)
		{
			PlayerController pc = GetGame().GetPlayerController();
			bool canPlayerRespawn = (pc && pc.CanRequestRespawn());
			int time = GetPlayerRemainingRespawnTime(m_iPlayerId);

			bool deployEnabled = (
				time == 0
				&& m_bSpawnPointsAvailable
				&& canPlayerRespawn
				&& m_bButtonsUnlocked
			);

			bool quickDeployEnabled = deployEnabled;
			bool respawnEnabled = m_RespawnSystemComponent.IsRespawnEnabled();

			bool focusedTile = true;
			if (m_TileSelection)
				focusedTile = m_TileSelection.GetFocusedTile();

			if (m_QuickDeployButton)
				m_QuickDeployButton.SetEnabled(respawnEnabled && m_bQuickDeployAvailable);
			if (m_ConfirmButton)
			{
				bool enable = true;
				if (m_bIsLastAvailableTab)
					enable = m_bSpawnPointsAvailable;
				m_ConfirmButton.SetEnabled(respawnEnabled && focusedTile && enable && m_bConfirmButtonEnabled);
			}
			if (!respawnEnabled)
				return;

			if (!m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId))
			{
				quickDeployEnabled = (
					time == 0
					&& m_RespawnSystemComponent.IsRespawnEnabled()
					&& m_bQuickDeployAvailable
				);
			}

			if (time > 0)
			{
				if (m_QuickDeployButton)
					m_QuickDeployButton.SetLabel(string.Format("%1 (%2)", m_sQuickDeploy, time));
				if (time != m_iLastTime)
				{
					SCR_UISoundEntity.SetSignalValueStr("countdownValue", time);
					SCR_UISoundEntity.SetSignalValueStr("maxCountdownValue", m_Timer.GetRespawnTime());
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN);
					if (SCR_SelectSpawnPointSubMenu.GetInstance()) // todo(koudelkaluk): temp
					{
						string timeFormatted = SCR_Global.GetTimeFormattingMinutesSeconds(0, time);
						SCR_SelectSpawnPointSubMenu.GetInstance().SetDeployCountdown(true, timeFormatted);
					}
					m_iLastTime = time;
				}
			}
			else
			{
				if (m_iLastTime == 1)
				{
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN_END);
					m_iLastTime = time;
					SetQuickDeployAvailable();
				}

				if (SCR_SelectSpawnPointSubMenu.GetInstance() && !m_bCountdownHidden)
				{
					string timeFormatted = SCR_Global.GetTimeFormattingMinutesSeconds(0, time);
					SCR_SelectSpawnPointSubMenu.GetInstance().SetDeployCountdown(false);
				}

				m_ConfirmButton.SetLabel(m_sConfirmButtonText);
				if (m_QuickDeployButton)
					m_QuickDeployButton.SetLabel(m_sQuickDeploy);

				if (m_ConfirmButton)
				{
					bool toggled = m_ConfirmButton.IsToggled();
					SCR_RespawnSuperMenu.Cast(m_ParentMenu).SetLoadingVisible(toggled);
					if (toggled && m_bIsLastAvailableTab && ConfirmSelection())
					{
						HandleOnDeploy();
					}
				}

				if (m_QuickDeployButton)
				{
					bool toggled = m_QuickDeployButton.IsToggled();
					SCR_RespawnSuperMenu.Cast(m_ParentMenu).SetLoadingVisible(toggled);
					if (quickDeployEnabled && s_bPlayableFactionsAvailable && toggled)
					{
						QuickDeploy();
					}
				}

#ifdef ENABLE_DIAG
				// OOF.
				if (SCR_RespawnHandlerComponent.IsCLISpawnEnabled())
				{
					SCR_RespawnHandlerComponent.UseCLISpawn();
					SCR_BaseGameMode gm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
					if (gm && gm.CanPlayerRespawn(pc.GetPlayerId()))
						HandleOnDeploy();
				}
#endif
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	protected void QuickDeploy()
	{
		if (m_bDeployRequestSent)
			return;
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(m_iPlayerId));
		if (rc)
			rc.RequestQuickRespawn();

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			groupsManager.SetConfirmedByPlayer(true);
		m_bDeployRequestSent = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateQuickDeployButton()
	{
		SCR_RespawnMenuHandlerComponent rmh = SCR_RespawnSuperMenu.Cast(m_ParentMenu).GetRespawnMenuHandler();
		if (!rmh.GetAllowQuickDeploy())
			return;

		m_QuickDeployButton = CreateNavigationButton("MenuQuickDeploy", m_sQuickDeploy, true);
		if (m_QuickDeployButton)
		{
			m_QuickDeployButton.GetRootWidget().SetZOrder(0);
			m_QuickDeployButton.SetToggleable(true);
			m_QuickDeployButton.m_OnToggled.Insert(ToggleQuickDeploy);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleQuickDeploy(SCR_ButtonBaseComponent btn, bool toggled)
	{
		SCR_RespawnSuperMenu rm = SCR_RespawnSuperMenu.Cast(m_ParentMenu);
		rm.ToggleQuickDeploy(toggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateConfirmButton()
	{
		m_ConfirmButton = CreateNavigationButton("MenuSelect", m_sConfirm, true);
		if (m_ConfirmButton)
		{
			m_ConfirmButton.m_OnActivated.Insert(HandleOnConfirm);
			m_ConfirmButton.GetRootWidget().SetZOrder(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ConfirmSelection()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleOnConfirm()
	{
		if (m_bIsLastAvailableTab && GetPlayerRemainingRespawnTime(m_iPlayerId) > 0)
			return;
		ConfirmSelection();
	}

	//------------------------------------------------------------------------------------------------
	private void HandleOnDeploy()
	{
		if (m_bDeployRequestSent)
			return;

		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(m_iPlayerId));
		if (rc)
			rc.RequestRespawn();

		m_bDeployRequestSent = true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool RequestFaction(Faction faction)
	{
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(m_iPlayerId));
		return rc.RequestPlayerFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	protected bool RequestLoadout(SCR_BasePlayerLoadout loadout)
	{
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(m_iPlayerId));
		return rc.RequestPlayerLoadout(loadout);
	}

	//------------------------------------------------------------------------------------------------
	protected bool RequestSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(m_iPlayerId));
		return rc.RequestPlayerSpawnPoint(spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	// disable deploy button if there are no factions or spawn points available
	protected void SetDeployAvailable()
	{
		Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
		if (!playerFaction)
			return;

		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(playerFaction.GetFactionKey());
		m_bSpawnPointsAvailable = !spawnPoints.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetQuickDeployAvailable()
	{
		if (!m_FactionManager)
			return;
		// check if any of the factions has valid spawn points
		array<Faction> factions = {};
		int cnt = m_FactionManager.GetFactionsList(factions);
		bool enable;

		for (int i = 0; i < cnt; ++i)
		{
			SCR_Faction faction = SCR_Faction.Cast(factions[i]);
			if (!faction.IsPlayable())
				continue;
			enable |= !SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey()).IsEmpty();
		}
		m_bQuickDeployAvailable = enable;
	}

	//------------------------------------------------------------------------------------------------
	bool GetConfirmButtonEnabled()
	{
		return m_bConfirmButtonEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetConfirmButtonEnabled(bool enabled)
	{
		m_bConfirmButtonEnabled = enabled;
	}
	
	SCR_NavigationButtonComponent GetConfirmButton()
	{
		return m_ConfirmButton;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_RespawnMenuHandlerComponent GetRespawnMenuHandler()
	{
		return SCR_RespawnSuperMenu.Cast(m_ParentMenu).GetRespawnMenuHandler();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RespawnSubMenuBase()
	{
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(SetDeployAvailable);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(SetQuickDeployAvailable);
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(SetQuickDeployAvailable);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RespawnSubMenuBase()
	{
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(SetDeployAvailable);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(SetQuickDeployAvailable);
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(SetQuickDeployAvailable);

		// m_bSpawnPointsAvailable = false;
	}
};