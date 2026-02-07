class SCR_PlayerXPHandlerComponentClass : ScriptComponentClass
{
}

//! Takes care of player-specific XP handling
//! Should be hooked on PlayerController
class SCR_PlayerXPHandlerComponent : ScriptComponent
{
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected int m_iPlayerXP = 0;

	protected int m_iPlayerXPSinceLastSpawn;

	protected float m_fSuicidePenaltyTimestamp;

	protected ref ScriptInvoker m_OnXPChanged;

	//------------------------------------------------------------------------------------------------
	//! Getter for player XP
	int GetPlayerXP()
	{
		return m_iPlayerXP;
	}

	//------------------------------------------------------------------------------------------------
	//! Setter for player XP accumulated since last respawn
	//! \param[in] xp
	void SetPlayerXPSinceLastSpawn(int xp)
	{
		m_iPlayerXPSinceLastSpawn = xp;
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for player XP accumulated since last respawn
	int GetPlayerXPSinceLastSpawn()
	{
		return m_iPlayerXPSinceLastSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetSuicidePenaltyTimestamp(float timestamp)
	{
		m_fSuicidePenaltyTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSuicidePenaltyTimestamp()
	{
		return m_fSuicidePenaltyTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnXPChanged()
	{
		if (!m_OnXPChanged)
			m_OnXPChanged = new ScriptInvoker();

		return m_OnXPChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsProxy()
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		return (rpl && rpl.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerKilled()
	{
		m_iPlayerXPSinceLastSpawn = 0;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] notify
	void UpdatePlayerRank(bool notify = true)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());

		if (!playerController)
			return;

		IEntity player = playerController.GetMainEntity();

		if (!player)
			return;

		SCR_CharacterRankComponent comp = SCR_CharacterRankComponent.Cast(player.FindComponent(SCR_CharacterRankComponent));

		if (!comp)
			return;

		SCR_ECharacterRank curRank = comp.GetCharacterRank(player);
		SCR_ECharacterRank newRank = SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetRankByXP(m_iPlayerXP);

		if (newRank == curRank)
			return;

		comp.SetCharacterRank(newRank, !notify);
	}

	//------------------------------------------------------------------------------------------------
	//! Cheat method to change player's rank - server side
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CheatRank(int playerID, bool demote)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		SCR_ECharacterRank rank = factionManager.GetRankByXP(m_iPlayerXP);
		int reqXP;

		if (demote)
		{
			SCR_ECharacterRank newRank = factionManager.GetRankPrev(rank);

			if (newRank == SCR_ECharacterRank.INVALID)
				return;

			reqXP = factionManager.GetRequiredRankXP(newRank) - m_iPlayerXP;
		}
		else
		{
			SCR_ECharacterRank newRank = factionManager.GetRankNext(rank);

			if (newRank == SCR_ECharacterRank.INVALID)
				return;

			reqXP = factionManager.GetRequiredRankXP(newRank) - m_iPlayerXP;
		}

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (!comp)
			return;

		comp.AwardXP(playerID, SCR_EXPRewards.CHEAT, reqXP);
	}

	//------------------------------------------------------------------------------------------------
	//! Cheat method to change player's rank
	void CheatRank(bool demote = false)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();
		Rpc(RpcAsk_CheatRank, playerID, demote);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_OnPlayerXPChanged(int currentXP, int XPToAdd, bool volunteer, SCR_EXPRewards rewardID, bool profileUsed, int skillLevel)
	{
		if (m_OnXPChanged)
			m_OnXPChanged.Invoke(currentXP, rewardID, XPToAdd, volunteer, profileUsed, skillLevel);
	}

	//------------------------------------------------------------------------------------------------
	//! Addition to player XP
	//! \param[in] rewardID
	//! \param[in] multiplier
	//! \param[in] volunteer
	//! \param[in] addDirectly
	void AddPlayerXP(SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int addDirectly = 0)
	{
		if (IsProxy())
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		
		if (!comp)
			return;

		if (addDirectly != 0)
		{
			m_iPlayerXP += (addDirectly);
			Replication.BumpMe();
			UpdatePlayerRank(false);
			Rpc(RpcDo_OnPlayerXPChanged, m_iPlayerXP, addDirectly, false, SCR_EXPRewards.UNDEFINED, false, 0);
			return;
		}

		int XP = comp.GetXPRewardAmount(rewardID);

		if (XP == 0)
			return;

		//EProfileSkillID skillID = comp.GetXPRewardSkill(rewardID);
		//auto profileManager = campaign.FindComponent(SCR_PlayerProfileManagerComponent); Replaced by SCR_PlayerData

		// 35% XP bonus when the player volunteered for the task
		if (volunteer)
			multiplier += 0.35;

		int XPToAdd = Math.Round(XP * multiplier * comp.GetXPMultiplier());

		const int XPToAddBySkill = 0; // TODO: check for good const usage
		bool profileUsed = false;
		const int skillLevel = 0; // TODO: check for good const usage
		/***** Replaced by SCR_PlayerData
		//****
		// Handle skill XP
		if (profileManager && XP > 0)
		{
			SCR_PlayerProfileManagerComponent profileManagerCast = SCR_PlayerProfileManagerComponent.Cast(profileManager);
			CareerBackendData profile = profileManagerCast.GetPlayerProfile(m_PlayerController.GetPlayerId());

			if (profile)
			{
				profileUsed = true;
				skillLevel = Math.Min(Math.Floor(profile.GetSkillXP(skillID) / SCR_GameModeCampaign.SKILL_LEVEL_XP_COST), SCR_GameModeCampaign.SKILL_LEVEL_MAX);
				XPToAddBySkill = Math.Round(XPToAdd * skillLevel * SCR_GameModeCampaign.SKILL_LEVEL_XP_BONUS);
				profile.AddSkillXP(skillID, XP * multiplier);
			}
		}
		//****
		*****/
		if (XPToAdd + XPToAddBySkill == 0)
			return;

		m_iPlayerXP += (XPToAdd + XPToAddBySkill);

		if (rewardID != SCR_EXPRewards.VETERANCY)
			m_iPlayerXPSinceLastSpawn += (XPToAdd + XPToAddBySkill);

		Replication.BumpMe();
		UpdatePlayerRank();

		Rpc(RpcDo_OnPlayerXPChanged, m_iPlayerXP, XPToAdd + XPToAddBySkill, volunteer, rewardID, profileUsed, skillLevel);
	}
}
