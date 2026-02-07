[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of loading and storing player profile data.", color: "0 0 255 255")]
class SCR_PlayerProfileManagerComponentClass: SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerProfileManagerComponent : SCR_BaseGameModeComponent
{
	//************************//
	//RUNTIME STATIC VARIABLES//
	//************************//
	protected static SCR_RespawnSystemComponent s_RespawnSystemComponent = null;
	protected static BackendApi s_BackendApi = null;
	
	//*****************//
	//MEMBER ATTRIBUTES//
	//*****************//
	[Attribute("1", "Refresh time for profile loading. [s]")]
	protected float m_fRefreshTime;
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected ref map<int, ref CareerBackendData> m_mPlayerProfiles = null;
	protected ref array<int> m_aPlayerIDsToLoadProfile = new ref array<int>();
	protected float m_fCurrentRefreshTime = 1;
	protected ref CampaignCallback m_Callback = new ref CampaignCallback();
	
	//------------------------------------------------------------------------------------------------
	protected Faction GetPlayerFaction(int playerID)
	{
		if (s_RespawnSystemComponent)
			return s_RespawnSystemComponent.GetPlayerFaction(playerID);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//Get method for player profiles in the m_mPlayerProfiles map
	CareerBackendData GetPlayerProfile(int playerID)
	{
		if (m_mPlayerProfiles)
			return m_mPlayerProfiles.Get(playerID);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a faction
	override void HandleOnFactionAssigned(int playerID, Faction assignedFaction)
	{
		if (!assignedFaction)
			return;
		
		CareerBackendData playerProfile = GetPlayerProfile(playerID);
		
		if (!playerProfile)
			return;
		
		playerProfile.SetFaction(assignedFaction.GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killer Entity of killer instigator if any.
	*/
	protected override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		CareerBackendData victimProfile = GetPlayerProfile(playerId);
		int killerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(killer);
		
		// Victim profile exists
		if (victimProfile)
		{
			// Add death no matter what
			victimProfile.AddDeath();
			
			// Suicide?
			if (playerId == killerId)
			{
				// Return, the rest of the code is irrelevant in this case, don't add kill to anyone
				return;
			}
		}
		
		CareerBackendData killerProfile = GetPlayerProfile(killerId);
		Faction victimFaction = GetPlayerFaction(playerId);
		Faction killerFaction = GetPlayerFaction(killerId);
		
		// Killer profile exists
		if (killerProfile)
		{
			// Both killer & victim factions exist
			if (killerFaction && victimFaction)
			{
				// Check faction friendliness
				if (killerFaction.IsFactionFriendly(victimFaction))
				{
					// Bad luck, the factions were friendly, add team kill
					killerProfile.AddKill(true);
					
					// Return, we don't want to add a regular kill to this team-killing monster
					return;
				}
			}
			
			// It wasn't a team kill, add a regular kill
			killerProfile.AddKill();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player disconnects.
	//! Method is called from SCR_DeathmatchLobbyEntity
	//! \param playerID is a unique player identifier that defines which player has disconnected.
	override void OnPlayerDisconnected(int playerId)
	{
		StoreProfile(playerId, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreProfile(int playerID, bool disconnecting = false)
	{
		if (!s_BackendApi)
			return;
		
		CareerBackendData playerProfile = GetPlayerProfile(playerID);
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetCampaign();
		
		if (!playerProfile || !m_Callback || !campaign)
			return;
		
		if (disconnecting)
			playerProfile.SetLogoutTime();
		
		#ifndef WORKBENCH
			s_BackendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S,m_Callback,playerProfile,playerID);
		#else
			s_BackendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate,m_Callback,playerProfile,playerID);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	bool LoadPlayerProfileFromBackend(int playerID)
	{
		if (m_mPlayerProfiles && s_BackendApi)
		{
			if (s_BackendApi.GetDSSession() && s_BackendApi.GetDSSession().Status() == EDsSessionState.EDSESSION_ACTIVE)
			{
				CareerBackendData playerProfile = new ref CareerBackendData();
				m_mPlayerProfiles.Set(playerID, playerProfile);
				playerProfile = GetPlayerProfile(playerID);
				
				if (m_Callback)
					s_BackendApi.PlayerData(playerProfile, playerID);
				
				return true;
			}
			else
			{
				return false;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadConnectingPlayerProfile(int playerID)
	{
		if (!LoadPlayerProfileFromBackend(playerID))
			m_aPlayerIDsToLoadProfile.Insert(playerID);
		else
		{
			CareerBackendData playerProfile = GetPlayerProfile(playerID);
			
			if (!playerProfile)
				return;
			
			playerProfile.SetLoginTime();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fCurrentRefreshTime -= timeSlice;
		
		if (m_fCurrentRefreshTime > 0)
			return;
		
		m_fCurrentRefreshTime = m_fRefreshTime;
		
		for (int count = m_aPlayerIDsToLoadProfile.Count(), i = count - 1; i >= 0; i--)
		{
			bool success = LoadPlayerProfileFromBackend(m_aPlayerIDsToLoadProfile[i]);
			
			if (success)
			{
				CareerBackendData playerProfile = GetPlayerProfile(m_aPlayerIDsToLoadProfile[i]);
				
				if (!playerProfile)
					return;
				
				playerProfile.SetLoginTime();
				m_aPlayerIDsToLoadProfile.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_fCurrentRefreshTime = m_fRefreshTime;
		m_mPlayerProfiles = new ref map<int, ref CareerBackendData>();
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(owner.FindComponent(SCR_RespawnSystemComponent));
		
		if (!respawnSystem)
		{
			Print("There is no RespawnSystemComponent attached to the GameMode entity. Faction scoring will not work.", LogLevel.WARNING);
			return;
		}
		
		s_RespawnSystemComponent = respawnSystem;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PlayerProfileManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (GetGame())
			s_BackendApi = GetGame().GetBackendApi();	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerProfileManagerComponent()
	{
		if (m_mPlayerProfiles)
		{
			m_mPlayerProfiles.Clear();
			m_mPlayerProfiles = null;
		}
		
		m_Callback = null;
	}

};
