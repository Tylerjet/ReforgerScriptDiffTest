#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_CampaignFeedbackComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignFeedbackComponent : ScriptComponent
{
	protected SCR_GameModeCampaign m_Campaign;

	protected SCR_PlayerController m_PlayerController;

	protected SCR_CampaignMilitaryBaseComponent m_BaseWithPlayer;

	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;

	protected vector m_vFirstSpawnPosition;

	protected SCR_MapCampaignUI m_MapCampaignUI;

	protected ref TimeContainer m_SpawnTime;

	protected bool m_bIsPlayerInRadioRange = true;
	protected bool m_bXPInfoOnKillsHintShown;

	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fBaseWithPlayerCaptureStart;
	protected float m_fBaseWithPlayerCaptureEnd;
	#else
	protected WorldTimestamp m_fBaseWithPlayerCaptureStart;
	protected WorldTimestamp m_fBaseWithPlayerCaptureEnd;
	#endif

	protected static const int HINTS_LIMIT_SUPPLIES = 5;
	protected static const int HINTS_LIMIT_SEIZING = 5;
	protected static const float ICON_FLASH_DURATION = 20;
	protected static const float ICON_FLASH_PERIOD = 0.5;

	protected bool m_bStartupHintsShown;
	protected bool m_bRespawnHintShown;
	protected bool m_bReinforcementsHintShown;
	protected bool m_bTicketsHintShown;
	protected bool m_bBaseLostHintShown;
	protected bool m_bCanShowSpawnPosition;
	protected bool m_bWasMapOpened;

	protected int m_iBaseSeizingHintsShown;
	protected int m_iSuppliesHintsShown;

	//------------------------------------------------------------------------------------------------
	static SCR_CampaignFeedbackComponent GetInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return SCR_CampaignFeedbackComponent.Cast(pc.FindComponent(SCR_CampaignFeedbackComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Serves for enabling spawn hint on map. Also saves player spawn position
	void EnablePlayerSpawnHint(bool enable)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();

		if (!player && enable)
			return;

		m_bCanShowSpawnPosition = enable;

		if (enable)
		{
			m_vFirstSpawnPosition = player.GetOrigin();
			SetSpawnTime();
		}
		else
		{
			m_vFirstSpawnPosition = vector.Zero;
			SetMapOpened(false);

			if (m_MapCampaignUI)
				m_MapCampaignUI.RemoveSpawnPositionHint();
		}

		GetGame().GetCallqueue().Remove(EnablePlayerSpawnHint);
	}

	//------------------------------------------------------------------------------------------------
	void SetMapCampaignUI(SCR_MapCampaignUI mapUi)
	{
		m_MapCampaignUI = mapUi;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnTime()
	{
		TimeAndWeatherManagerEntity manager = GetGame().GetTimeAndWeatherManager();

		if (manager)
				m_SpawnTime = manager.GetTime();
		else
			Print("Time And Weather manager not found", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	TimeContainer GetSpawnTime()
	{
		return m_SpawnTime;
	}

	//------------------------------------------------------------------------------------------------
	bool CanShowPlayerSpawn()
	{
		return m_bCanShowSpawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns players position after spawning
	vector GetPlayerSpawnPos()
	{
		return m_vFirstSpawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if map was already opened
	bool WasMapOpened()
	{
		return m_bWasMapOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets that map was already opened by player
	void SetMapOpened(bool wasOpened)
	{
		m_bWasMapOpened = wasOpened;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetBaseWithPlayer()
	{
		return m_BaseWithPlayer;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckPlayerInsideRadioRange()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();

		if (!player)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!faction)
			return;

		bool isInRangeNow = m_Campaign.GetBaseManager().IsEntityInFactionRadioSignal(player, faction);

		if (isInRangeNow != m_bIsPlayerInRadioRange)
		{
			m_bIsPlayerInRadioRange = isInRangeNow;

			if (isInRangeNow)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeEntered-UC", duration: 3);
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_SIGNAL_GAIN);
			}
			else
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeLeft-UC", duration: 3);
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_SIGNAL_LOST);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterTasksShown()
	{
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
		GetGame().GetInputManager().RemoveActionListener("GadgetMap", EActionTrigger.DOWN, RegisterTasksShown);
	}

	//------------------------------------------------------------------------------------------------
	void SetIsPlayerInRadioRange(bool status)
	{
		m_bIsPlayerInRadioRange = status;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshCurrentPopupMessage()
	{
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		SCR_PopupMessage currentMsg = popup.GetCurrentMsg();

		// Player is not currently in any base, hide any relevant displayed popups
		if (!m_BaseWithPlayer)
		{
			if (currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
				popup.HideCurrentMsg();
		}
		else
		{
			SCR_CampaignFaction baseWithPlayerCapturingFaction = SCR_CampaignFaction.Cast(m_BaseWithPlayer.GetCapturingFaction());

			if (!baseWithPlayerCapturingFaction)
			{
				// Player's base is currently not being seized by anyone, hide any relevant displayed popups
				if (currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
					popup.HideCurrentMsg();
			}
			else
			{
				// If relay tower is being reconfigured, calculate the capture duration
				if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.RELAY)
					#ifndef AR_CAMPAIGN_TIMESTAMP
					m_fBaseWithPlayerCaptureEnd = m_fBaseWithPlayerCaptureStart + SCR_CampaignMilitaryBaseComponent.RADIO_RECONFIGURATION_DURATION * 1000;
					#else
					m_fBaseWithPlayerCaptureEnd = m_fBaseWithPlayerCaptureStart.PlusSeconds(SCR_CampaignMilitaryBaseComponent.RADIO_RECONFIGURATION_DURATION);
					#endif

				// If capture timers are invalid, hide seizing popup
				if (m_fBaseWithPlayerCaptureStart == 0 || m_fBaseWithPlayerCaptureEnd == 0 && currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
				{
					popup.HideCurrentMsg();
				}
				else
				{
					if (baseWithPlayerCapturingFaction == SCR_FactionManager.SGetLocalPlayerFaction())
					{
						// Friendlies are seizing player's base
						if (currentMsg && currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY)
							popup.HideCurrentMsg();

						if (m_BaseWithPlayer.GetReconfiguredByID() != SCR_PlayerController.GetLocalPlayerId())
						{
							if (!currentMsg || currentMsg.m_iPriority != SCR_ECampaignSeizingMessagePrio.SEIZING_YOU)
								popup.PopupMsg("#AR-Campaign_SeizingFriendly-UC", -1, prio: SCR_ECampaignSeizingMessagePrio.SEIZING_YOU, progressStart: m_fBaseWithPlayerCaptureStart, progressEnd: m_fBaseWithPlayerCaptureEnd, category: SCR_EPopupMsgFilter.TUTORIAL);
						}
					}
					else
					{
						// Enemies are seizing player's base
						if (currentMsg && currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU)
							popup.HideCurrentMsg();

						if (!currentMsg || currentMsg.m_iPriority != SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY)
							popup.PopupMsg("#AR-Campaign_SeizingEnemy-UC", -1, prio: SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY, progressStart: m_fBaseWithPlayerCaptureStart, progressEnd: m_fBaseWithPlayerCaptureEnd, category: SCR_EPopupMsgFilter.TUTORIAL);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowHint(SCR_ECampaignHints hintID)
	{
		if (m_Campaign.IsTutorial())
			return;

		switch (hintID)
		{
			case SCR_ECampaignHints.SIGNAL:
			{
				if (!m_bStartupHintsShown)
				{
					m_bStartupHintsShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Signal_Text", "#AR-Campaign_GamemodeName", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_OVERVIEW);
					GetGame().GetCallqueue().CallLater(ShowHint, 32000, false, SCR_ECampaignHints.SERVICES);
				}
				else
				{
					if (!m_bRespawnHintShown)
					{
						m_bRespawnHintShown = true;
						ShowHint(SCR_ECampaignHints.RESPAWN);
					}
				}

				return;
			}

			case SCR_ECampaignHints.SERVICES:
			{
				string hintString = "#AR-Campaign_Hint_ServicesText <h1 align='center' scale='4'><color rgba='0,112,20,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Supplies'/></color></h1>";
				SCR_HintManagerComponent.ShowCustomHint(hintString, "#AR-Campaign_Hint_Services_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_SERVICES);
				return;
			}

			case SCR_ECampaignHints.RESPAWN:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Respawn_Text", "#AR-Campaign_Hint_Respawn_Title", 20, fieldManualEntry: EFieldManualEntryId.CONFLICT_RESPAWN);
				return;
			}

			case SCR_ECampaignHints.SEIZING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_EnteringEnemyBase_Presence_Text", "#AR-Campaign_EnteringEnemyBase", 15, fieldManualEntry: EFieldManualEntryId.CONFLICT_SEIZING_BASES);
				m_iBaseSeizingHintsShown++;
				return;
			}

			case SCR_ECampaignHints.RECONFIGURING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_EnteringEnemyRelay_Text", "#AR-EditableEntity_TransmitterTower_01_Name", 15);
				return;
			}

			case SCR_ECampaignHints.SUPPLY_RUNS:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRun_Text", "#AR-Campaign_Hint_SupplyRun_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
				return;
			}

			case SCR_ECampaignHints.SUPPLIES_UNLOADING:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRunUnload_Text", "#AR-Campaign_Hint_SupplyRun_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
				m_iSuppliesHintsShown++;
				return;
			}

			case SCR_ECampaignHints.KILL_XP:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_KillXP_Text", "#AR-Campaign_Hint_KillXP_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_RANKS);
				return;
			}

			case SCR_ECampaignHints.MOBILE_ASSEMBLY:
			{
				SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_MobileAssembly_Text", "#AR-Vehicle_MobileAssembly_Name", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_MHQ);
				return;
			}

			case SCR_ECampaignHints.REINFORCEMENTS:
			{
				if (!m_bReinforcementsHintShown)
				{
					m_bReinforcementsHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Reinforcements_Text", "#AR-Campaign_Hint_Reinforcements_Title", 20, fieldManualEntry: EFieldManualEntryId.CONFLICT_REINFORCEMENTS);
				}

				return;
			}

			case SCR_ECampaignHints.TICKETS:
			{
				if (!m_bTicketsHintShown)
				{
					m_bTicketsHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_Supplies_Text", "#AR-Campaign_Hint_Services_Title", 30, fieldManualEntry: EFieldManualEntryId.CONFLICT_RESPAWN);
				}

				return;
			}

			case SCR_ECampaignHints.BASE_LOST:
			{
				if (!m_bBaseLostHintShown)
				{
					m_bBaseLostHintShown = true;
					SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_BaseLost_Text", "#AR-Campaign_Hint_BaseLost_Title", 15, fieldManualEntry: EFieldManualEntryId.CONFLICT_SEIZING_BASES);
				}

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		if (player != SCR_PlayerController.GetLocalControlledEntity())
			return;

		if (m_BaseWithPlayer)
			OnBaseLeft(m_BaseWithPlayer);

		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseFactionChanged(notnull SCR_MilitaryBaseComponent base, Faction faction)
	{
		IEntity player;
		
		if (m_PlayerController)
			player = m_PlayerController.GetControlledEntity();

		if (player && faction == SCR_FactionManager.SGetLocalPlayerFaction())
			ShowHint(SCR_ECampaignHints.TICKETS);
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseEntered(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (m_BaseWithPlayer)
			OnBaseLeft(m_BaseWithPlayer);

		m_BaseWithPlayer = base;

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(m_BaseWithPlayer.GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
		{
			OnSeizingTimerChange(seizingComponent.GetSeizingStartTimestamp(), seizingComponent.GetSeizingEndTimestamp());
			seizingComponent.GetOnTimerChange().Remove(OnSeizingTimerChange);
			seizingComponent.GetOnTimerChange().Insert(OnSeizingTimerChange);
		}
		else
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			OnSeizingTimerChange(0, 0);
			#else
			OnSeizingTimerChange(null, null);
			#endif
		}

		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (m_BaseWithPlayer.GetFaction() != playerFaction)
		{
			if ((m_iBaseSeizingHintsShown < HINTS_LIMIT_SEIZING || m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.RELAY) && m_BaseWithPlayer.IsHQRadioTrafficPossible(playerFaction))
			{
				if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.RELAY)
					ShowHint(SCR_ECampaignHints.RECONFIGURING);
				else
					ShowHint(SCR_ECampaignHints.SEIZING);
			}
		}
		else if (m_iSuppliesHintsShown < HINTS_LIMIT_SUPPLIES)
		{
			// Entering a friendly base in a supply truck
			ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

			if (!player || !player.IsInVehicle())
				return;

			CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(player.FindComponent(CompartmentAccessComponent));

			if (!compartmentAccessComponent)
				return;

			BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();

			if (!compartmentSlot)
				return;

			Vehicle vehicle = Vehicle.Cast(compartmentSlot.GetOwner());

			if (!vehicle)
				return;

			SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(vehicle.FindComponent(SCR_CampaignSuppliesComponent));

			if (!suppliesComponent)
				return;

			if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.BASE && !m_BaseWithPlayer.IsHQ())
			{
				if (suppliesComponent.GetSupplies() != 0)
					ShowHint(SCR_ECampaignHints.SUPPLIES_UNLOADING);
			}
			else
			{
				if (m_BaseWithPlayer.GetType() != SCR_ECampaignBaseType.RELAY)
				{
					if (suppliesComponent.GetSupplies() != suppliesComponent.GetSuppliesMax())
					{
						m_iSuppliesHintsShown++;
						SCR_HintManagerComponent.ShowCustomHint("#AR-Campaign_Hint_SupplyRunLoad_Text", "#AR-Campaign_Hint_SupplyRun_Title", 10, fieldManualEntry: EFieldManualEntryId.CONFLICT_SUPPLIES);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseLeft(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
			seizingComponent.GetOnTimerChange().Remove(OnSeizingTimerChange);

		if (m_BaseWithPlayer == base)
			m_BaseWithPlayer = null;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void OnSeizingTimerChange(float start, float end)
	#else
	void OnSeizingTimerChange(WorldTimestamp start, WorldTimestamp end)
	#endif
	{
		m_fBaseWithPlayerCaptureStart = start;
		m_fBaseWithPlayerCaptureEnd = end;
		SCR_PopUpNotification.GetInstance().ChangeProgressBarFinish(end);
	}

	//------------------------------------------------------------------------------------------------
	void MobileAssemblyFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		Faction localPlayerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (fManager.GetFactionIndex(localPlayerFaction) != factionID)
			return;

		int duration = 6;
		LocalizedString text;
		LocalizedString text2;
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (mapEntity && mapEntity.IsOpen())
			m_MapCampaignUI.InitMobileAssembly(localPlayerFaction.GetFactionKey(), msgID == SCR_EMobileAssemblyStatus.DEPLOYED);

		switch (msgID)
		{
			case SCR_EMobileAssemblyStatus.DEPLOYED:
			{
				text = "#AR-Campaign_MobileAssemblyDeployed-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				break;
			}
			case SCR_EMobileAssemblyStatus.DISMANTLED:
			{
				text = "#AR-Campaign_MobileAssemblyDismantled-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				Faction reconfigurerFaction = SCR_FactionManager.SGetPlayerFaction(playerID);

				if (reconfigurerFaction && localPlayerFaction != reconfigurerFaction)
					playerName = reconfigurerFaction.GetFactionName();

				break;
			}
			case SCR_EMobileAssemblyStatus.DESTROYED:
			{
				text = "#AR-Campaign_MobileAssemblyDestroyed-UC";
				break;
			}
		}

		if (text != string.Empty)
		{
			if (text2 != string.Empty && playerName != string.Empty)
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, text2param1: playerName, prio: SCR_ECampaignPopupPriority.MHQ);
			else
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration);
		}
	}

	//------------------------------------------------------------------------------------------------
	void PlayRadioMsg(SCR_ERadioMsg msg, int factionId, int baseCallsign, int callerCallsignCompany, int callerCallsignPlatoon, int callerCallsignSquad, int calledCallsignCompany, int calledCallsignPlatoon, int calledCallsignSquad, int param, float seed, float quality)
	{
		if (m_Campaign.IsTutorial())
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (!pc)
			return;

		IEntity player = pc.GetMainEntity();

		if (!player)
			return;

		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(player.FindComponent(SCR_CommunicationSoundComponent));

		if (!soundComp)
			return;

		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(player.FindComponent(SignalsManagerComponent));

		if (!signalComp)
			return;

		int signalBase = signalComp.AddOrFindSignal("Base");
		int signalCompanyCaller = signalComp.AddOrFindSignal("CompanyCaller");
		int signalCompanyCalled = signalComp.AddOrFindSignal("CompanyCalled");
		int signalPlatoonCaller = signalComp.AddOrFindSignal("PlatoonCaller");
		int signalPlatoonCalled = signalComp.AddOrFindSignal("PlatoonCalled");
		int signalSquadCaller = signalComp.AddOrFindSignal("SquadCaller");
		int signalSquadCalled = signalComp.AddOrFindSignal("SquadCalled");
		int signalSeed = signalComp.AddOrFindSignal("Seed");
		int signalQuality = signalComp.AddOrFindSignal("TransmissionQuality");

		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(factionId));
		SCR_CampaignMilitaryBaseComponent base = m_Campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);

		if (base)
		{
			SCR_CampaignBaseCallsign callsignInfo;
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

			if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign());
			else
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign(), campaign.GetCallsignOffset());
	
			if (callsignInfo)
				signalComp.SetSignalValue(signalBase, callsignInfo.GetSignalIndex());
		}

		if (callerCallsignCompany != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			signalComp.SetSignalValue(signalCompanyCaller, callerCallsignCompany);
			signalComp.SetSignalValue(signalPlatoonCaller, callerCallsignPlatoon);
			signalComp.SetSignalValue(signalSquadCaller, callerCallsignSquad);
		}

		if (calledCallsignCompany != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			signalComp.SetSignalValue(signalCompanyCalled, calledCallsignCompany);
			signalComp.SetSignalValue(signalPlatoonCalled, calledCallsignPlatoon);
			signalComp.SetSignalValue(signalSquadCalled, calledCallsignSquad);
		}

		signalComp.SetSignalValue(signalSeed, seed);
		signalComp.SetSignalValue(signalQuality, quality);

		string msgName;
		LocalizedString text;
		LocalizedString text2;
		string param1;
		string text2param1;
		string text2param2;
		int duration = SCR_PopUpNotification.DEFAULT_DURATION;
		SCR_ECampaignPopupPriority prio = SCR_ECampaignPopupPriority.DEFAULT;
		string sound;

		switch (msg)
		{
			case SCR_ERadioMsg.SEIZED_MAIN:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_MOB;
				break;
			}

			case SCR_ERadioMsg.SEIZED_MAJOR:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_FOB;
				break;
			}

			case SCR_ERadioMsg.SEIZED_SMALL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_COP;
				break;
			}

			case SCR_ERadioMsg.DEMOTION_RENEGADE:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_REN;
				text = "#AR-Campaign_Demotion-UC";
				text2 = "#AR-Rank_Renegade";
				break;
			}

			case SCR_ERadioMsg.DEMOTION:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_DEM;
				text = "#AR-Campaign_Demotion-UC";
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

				if (f)
					text2 = f.GetRankNameUpperCase(param);

				break;
			}

			case SCR_ERadioMsg.PROMOTION_PRIVATE:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_POP;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestPrivate";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_CORPORAL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_POC;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCorporal";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_SERGEANT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_POS;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestSergeant";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_LIEUTENANT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_POL;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestLieutenant";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_CAPTAIN:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_PON;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCaptain";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_MAJOR:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_POM;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestMajor";
				break;
			}

			case SCR_ERadioMsg.VICTORY:
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));

				if (!f || f != SCR_FactionManager.SGetLocalPlayerFaction())
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_PMV;
				break;
			}

			case SCR_ERadioMsg.WINNING:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_PMC;
				break;
			}

			case SCR_ERadioMsg.LOSING:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_PML;
				break;
			}

			case SCR_ERadioMsg.DEFEAT:
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));

				if (!f || f == SCR_FactionManager.SGetLocalPlayerFaction())
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_PMD;
				break;
			}

			case SCR_ERadioMsg.RELAY:
			{
				msgName = SCR_SoundEvent.SOUND_SL_RRD;
				break;
			}

			case SCR_ERadioMsg.REQUEST_EVAC:
			{
				msgName = SCR_SoundEvent.SOUND_SL_ERT;
				break;
			}

			case SCR_ERadioMsg.REQUEST_FUEL:
			{
				msgName = SCR_SoundEvent.SOUND_SL_RRT;
				break;
			}

			case SCR_ERadioMsg.SUPPLIES:
			{
				msgName = SCR_SoundEvent.SOUND_SL_SDD;
				break;
			}

			case SCR_ERadioMsg.REQUEST_REINFORCEMENTS:
			{
				msgName = SCR_SoundEvent.SOUND_SL_REI;
				break;
			}

			case SCR_ERadioMsg.REQUEST_TRANSPORT:
			{
				msgName = SCR_SoundEvent.SOUND_SL_TRT;
				break;
			}

			case SCR_ERadioMsg.CONFIRM:
			{
				msgName = SCR_SoundEvent.SOUND_SL_CSR;
				break;
			}

			case SCR_ERadioMsg.TASK_ASSIGN_SEIZE:
			{
				msgName = SCR_SoundEvent.SOUND_SL_SRT;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_REFUEL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_RRU;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_TRANSPORT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_TRU;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_EVAC:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_ETU;
				break;
			}

			case SCR_ERadioMsg.TASK_CANCEL_REQUEST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_RCR;
				break;
			}

			case SCR_ERadioMsg.TASK_ASSIST:
			{
				msgName = SCR_SoundEvent.SOUND_SL_CHR;
				break;
			}

			case SCR_ERadioMsg.BASE_LOST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BAL;
				break;
			}

			case SCR_ERadioMsg.RELAY_LOST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_CTL;
				break;
			}

			case SCR_ERadioMsg.BASE_UNDER_ATTACK:
			{
				if (!base)
					return;

				if (base.GetType() == SCR_ECampaignBaseType.BASE)
					msgName = SCR_SoundEvent.SOUND_HQ_BUA;

				text = "#AR-Campaign_BaseUnderAttack-UC";

				if (base.GetType() == SCR_ECampaignBaseType.RELAY)
					text2 = "%1";
				else
					text2 = "%1 (%2)";

				text2param1 = base.GetBaseName();
				text2param2 = base.GetCallsignDisplayName();

				if (m_BaseWithPlayer == base)
					sound = SCR_SoundEvent.SOUND_SIREN;

				prio = SCR_ECampaignPopupPriority.BASE_UNDER_ATTACK;
				duration = 11;
				break;
			}

			case SCR_ERadioMsg.BUILT_ARMORY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BAA;
				break;
			}

			case SCR_ERadioMsg.BUILT_FUEL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BFA;
				break;
			}

			case SCR_ERadioMsg.BUILT_REPAIR:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BRA;
				break;
			}

			case SCR_ERadioMsg.BUILT_SUPPLY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_SCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_VEHICLES_LIGHT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_LCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_VEHICLES_HEAVY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_VCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_BARRACKS:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_ANTENNA:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_ACB;
				break;
			}

			case SCR_ERadioMsg.BUILT_FIELD_HOSPITAL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_HCB;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_ARMORY:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BAD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_FUEL:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BFD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_REPAIR:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BRD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_ARMORY:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BAR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_FUEL:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BFR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_REPAIR:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BRR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}
		}

		bool isFriendly = faction == fManager.GetPlayerFaction(pc.GetPlayerId());

		if (!msgName.IsEmpty())
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			msgName = msgName + "_" + faction.GetFactionKey();
			m_PlayedRadio = soundComp.SoundEvent(msgName);
		}

		if (isFriendly && (!text.IsEmpty() || !text2.IsEmpty()))
			SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, prio: prio, param1: param1, sound: sound, text2param1: text2param1, text2param2: text2param2);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnXPChanged(int totalXP, SCR_EXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		if (!m_bXPInfoOnKillsHintShown && (rewardID == SCR_EXPRewards.ENEMY_KILL || rewardID == SCR_EXPRewards.ENEMY_KILL_VEH))
		{
			m_bXPInfoOnKillsHintShown = true;
			GetGame().GetCallqueue().CallLater(ShowHint, 30000 + Math.RandomIntInclusive(0, 30000), false, SCR_ECampaignHints.KILL_XP);
		}
	}

	//------------------------------------------------------------------------------------------------
	void FlashBaseIcon(notnull SCR_CampaignMilitaryBaseComponent base, float remainingTime = ICON_FLASH_DURATION, Faction faction = null, bool changeToDefault = false, bool infiniteTimer = false)
	{
		GetGame().GetCallqueue().Remove(FlashBaseIcon);

		SCR_CampaignMapUIBase ui = base.GetMapUI();

		if (ui)
		{
			SCR_CampaignFaction baseFaction = base.GetCampaignFaction();
			Faction iconFaction;

			if (changeToDefault)
			{
				if (!baseFaction.IsPlayable() || base.IsHQRadioTrafficPossible(baseFaction))
					iconFaction = baseFaction;
			}
			else
			{
				if (faction && base.IsHQRadioTrafficPossible(baseFaction))
					iconFaction = faction;
				else
					return;
			}

			ui.SetBaseIconFactionColor(iconFaction);
		}

		float remainingTimeReduced = remainingTime - ICON_FLASH_PERIOD;

		if (infiniteTimer || remainingTimeReduced > 0 || !changeToDefault)
			GetGame().GetCallqueue().CallLater(FlashBaseIcon, ICON_FLASH_PERIOD * 1000, false, base, remainingTimeReduced, faction, !changeToDefault, infiniteTimer);
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessEvents(bool activate)
	{
		GetGame().GetCallqueue().Remove(CheckPlayerInsideRadioRange);
		GetGame().GetCallqueue().Remove(RefreshCurrentPopupMessage);

		if (activate)
		{
			GetGame().GetCallqueue().CallLater(CheckPlayerInsideRadioRange, 3000, true);
			GetGame().GetCallqueue().CallLater(RefreshCurrentPopupMessage, 500, true);

			GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
			GetGame().GetInputManager().AddActionListener("GadgetMap", EActionTrigger.DOWN, RegisterTasksShown);

			SCR_MilitaryBaseManager.GetInstance().GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);

			if (m_Campaign)
			{
				m_Campaign.GetOnPlayerKilled().Insert(OnPlayerKilled);
				m_Campaign.GetBaseManager().GetOnLocalPlayerEnteredBase().Insert(OnBaseEntered);
				m_Campaign.GetBaseManager().GetOnLocalPlayerLeftBase().Insert(OnBaseLeft);
			}

			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));

			if (comp)
				comp.GetOnXPChanged().Insert(OnXPChanged);
		}
		else
		{
			GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
			GetGame().GetInputManager().RemoveActionListener("GadgetMap", EActionTrigger.DOWN, RegisterTasksShown);

			SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance(false);

			if (baseManager)
				baseManager.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);

			if (m_Campaign)
			{
				m_Campaign.GetOnPlayerKilled().Remove(OnPlayerKilled);
				
				SCR_CampaignMilitaryBaseManager campaignBaseManager = m_Campaign.GetBaseManager();
				
				if (campaignBaseManager)
				{
					campaignBaseManager.GetOnLocalPlayerEnteredBase().Remove(OnBaseEntered);
					campaignBaseManager.GetOnLocalPlayerLeftBase().Remove(OnBaseLeft);
				}
			}

			if (m_PlayerController)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.GetOnXPChanged().Remove(OnXPChanged);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnOwnershipChanged(bool changing, bool becameOwner)
	{
		if (changing)
			return;

		ProcessEvents(becameOwner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_PlayerController)
			return;

		m_Campaign = SCR_GameModeCampaign.GetInstance();

		m_PlayerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		RplComponent rpl = RplComponent.Cast(m_PlayerController.FindComponent(RplComponent));

		if (rpl && rpl.IsOwner())
			ProcessEvents(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);

		m_PlayerController = SCR_PlayerController.Cast(owner);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignFeedbackComponent()
	{
		ProcessEvents(false);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_ECampaignHints
{
	NONE,
	SIGNAL,
	SERVICES,
	RESPAWN,
	SEIZING,
	RECONFIGURING,
	SUPPLY_RUNS,
	KILL_XP,
	MOBILE_ASSEMBLY,
	SUPPLIES_UNLOADING,
	REINFORCEMENTS,
	TICKETS,
	BASE_LOST
};

//------------------------------------------------------------------------------------------------
//! Popup message priorities sorted from lowest to highest
enum SCR_ECampaignPopupPriority
{
	DEFAULT,
	SUPPLIES_HANDLED,
	TASK_AVAILABLE,
	RELAY_DETECTED,
	TASK_PROGRESS,
	TASK_DONE,
	BASE_LOST,
	MHQ,
	BASE_UNDER_ATTACK,
	RESPAWN,
	MATCH_END
};

enum SCR_ECampaignSeizingMessagePrio
{
	NONE,
	SEIZING_YOU = 998,
	SEIZING_ENEMY = 999
};
