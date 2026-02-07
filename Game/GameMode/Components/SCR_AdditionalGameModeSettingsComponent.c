[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_AdditionalGameModeSettingsComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_AdditionalGameModeSettingsComponent : SCR_BaseGameModeComponent
{
	protected static SCR_AdditionalGameModeSettingsComponent s_Instance;
	
	[Attribute("1", desc: "If true team kill punishment is enabled, otherwise it is disabled", category: "Teamkilling"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bEnableTeamKillPunishment;
	
	[Attribute("1", desc: "If true allows for vehicles (and potential other entities) to be refunded at depods.", category: "Entity Refund"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bAllowEntityRefundingAction;
	
	[Attribute("#AR-Editor_Action_Disabled_By_GM", desc: "Shown on the entity action why refunding is disabled. The action is simply hidden if this is empty.", category: "Entity Refund")]
	protected LocalizedString m_sEntityRefundingDisabledReason;

	[Attribute("1", desc: "If true it will show additional information in form of ui element that shows ballistic data (f.e. in mortars).", category: "Entity Refund"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bProjectileBallisticInfoVisibility;

	[Attribute(desc: "If true artillery ai command will get position offset based on the distance from the player to the position at which player ordered the artillery")]
	protected bool m_bAdditionalArtilleryOrderDistancePenalty;

	protected ref ScriptInvokerVoid m_OnChangeAdditionalSettingsInvoker;

	//------------------------------------------------------------------------------------------------
	bool GetProjectileBallisticInfoVisibility()
	{
		return m_bProjectileBallisticInfoVisibility;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnChangeAdditionalSettingsInvoker()
	{
		if (!m_OnChangeAdditionalSettingsInvoker)
			m_OnChangeAdditionalSettingsInvoker = new ScriptInvokerVoid();

		return m_OnChangeAdditionalSettingsInvoker;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAdditionalSettingsChanged()
	{
		if (!m_OnChangeAdditionalSettingsInvoker)
			return;

		m_OnChangeAdditionalSettingsInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AdditionalGameModeSettingsComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! State of the scenario setting that dictates if game should add an additional offset to the position of the artillery command issued by the player
	//! \return true if position of the artillery command issued by the player should be spoofed with random offset
	bool IsAdditionalArtileryOrderDistancePenaltyEnabled()
	{
		return m_bAdditionalArtilleryOrderDistancePenalty;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets team kill punishment is enabled or disabled and notifies players if playerID is provided. (Server only)
	//! \param[in] enablePunishment Enable punishment setting for team kills, toggles team kill punishment on or off for all players
	//! \param[in] playerID Player ID is an optional parameter representing the ID of the GM changing the setting. Will send notification if changed
	void SetEnableTeamKillPunishment_S(bool enablePunishment, int playerID = -1)
	{
		if (m_bEnableTeamKillPunishment == enablePunishment || !GetGameMode().IsMaster())
			return;
		
		m_bEnableTeamKillPunishment = enablePunishment;
		
		if (playerID > 0)
		{
			if (m_bEnableTeamKillPunishment)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_TEAMKILL_PUNISHMENT_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_TEAMKILL_PUNISHMENT_DISABLED, playerID);
		}
			
		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether team killing punishment is enabled.
	bool IsTeamKillingPunished()
	{
		return m_bEnableTeamKillPunishment;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if players are allowed to refund entities or not. Note currently only vehicles can be refunded. (Server only)
	//! \param[in] allowRefunding Set if vehicles are allowed to be refunded
	//! \param[in] playerID Optional, will send a notification when a player (GM) changes this setting. No notifications are send if playerID is -1
	void SetAllowEntityRefundingAction_S(bool allowRefunding, int playerID = -1)
	{
		if (m_bAllowEntityRefundingAction == allowRefunding || !GetGameMode().IsMaster())
			return;
		
		m_bAllowEntityRefundingAction = allowRefunding;
		
		if (playerID > 0)
		{
			if (m_bAllowEntityRefundingAction)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_REFUND_ENTITY_AT_DEPOTS_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_REFUND_ENTITY_AT_DEPOTS_DISABLED, playerID);
		}
			
		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If players are allowed to refund entities
	bool IsEntityRefundingActionAllowed()
	{
		return m_bAllowEntityRefundingAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Disable reason for action when the player cannot refund. If empty will simply not show the action
	string GetEntityRefundingDisabledReason()
	{
		return m_sEntityRefundingDisabledReason;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if game should show ui elements that provide projectile ballistic data.
	//! \param[in] shouldShow Set if such ui elements should be visible
	//! \param[in] playerID Optional, will send a notification when a player (GM) changes this setting. No notifications are send if playerID is -1
	void SetProjectileBallisticInfoVisibility_S(bool shouldShow, int playerID = -1)
	{
		if (m_bProjectileBallisticInfoVisibility == shouldShow || !GetGameMode().IsMaster())
			return;
		
		m_bProjectileBallisticInfoVisibility = shouldShow;

		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AdditionalGameModeSettingsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (s_Instance)
		{
			Print("'SCR_AdditionalGameModeSettingsComponent' exists twice in the world!", LogLevel.WARNING);
			return;
		}
			
		s_Instance = this;
	}
	
}
