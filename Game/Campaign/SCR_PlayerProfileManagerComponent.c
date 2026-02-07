[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of loading and storing player profile data.", color: "0 0 255 255")]
class SCR_PlayerProfileManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PlayerProfileManagerComponent : SCR_BaseGameModeComponent
{
	//************************//
	//RUNTIME STATIC VARIABLES//
	//************************//
	protected static SCR_RespawnSystemComponent s_RespawnSystemComponent = null;
	
	//*****************//
	//MEMBER ATTRIBUTES//
	//*****************//
	[Attribute("1", "Refresh time for profile loading. [s]")]
	protected float m_fRefreshTime;
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected ref map<int, ref CareerBackendData> m_mPlayerProfiles = null;
	protected ref array<int> m_aPlayerIDsToLoadProfile = {};
	protected float m_fCurrentRefreshTime = 1;
	protected ref CampaignCallback m_Callback = new CampaignCallback();
	
	//------------------------------------------------------------------------------------------------
	protected Faction GetPlayerFaction(int playerID)
	{
		return SCR_FactionManager.SGetPlayerFaction(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return player profile from playerID
	CareerBackendData GetPlayerProfile(int playerID)
	{
		if (m_mPlayerProfiles)
			return m_mPlayerProfiles.Get(playerID);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player is assigned a faction
	//! \param[in] playerID
	//! \param[in] assignedFaction
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
	//! Called after a player gets killed.
	//! \param[in] playerId PlayerId of victim player.
	//! \param[in] playerEntity Entity of victim player if any.
	//! \param[in] killerEntity Entity of killer instigator if any.
	//! \param[in] killer Instigator of the kill, use type to see if there is any
	protected override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		
		CareerBackendData victimProfile = GetPlayerProfile(playerId);
		int killerId = killer.GetInstigatorPlayerID();
		
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
	//! \param[in] playerID is a unique player identifier that defines which player has disconnected.
	//! \param[in] cause
	//! \param[in] timeout
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		StoreProfile(playerId, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] disconnecting
	void StoreProfile(int playerID, bool disconnecting = false)
	{
		if (!GetGame().GetBackendApi())
			return;
		
		CareerBackendData playerProfile = GetPlayerProfile(playerID);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!playerProfile || !m_Callback || !campaign)
			return;
		
		if (disconnecting)
			playerProfile.SetLogoutTime();
		
		#ifndef WORKBENCH
			GetGame().GetBackendApi().PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S,m_Callback,playerProfile,playerID);
		#else
			GetGame().GetBackendApi().PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate,m_Callback,playerProfile,playerID);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \return
	bool LoadPlayerProfileFromBackend(int playerID)
	{
		if (m_mPlayerProfiles && GetGame().GetBackendApi())
		{
			if (GetGame().GetBackendApi().GetDSSession() && GetGame().GetBackendApi().GetDSSession().Status() == EDsSessionState.EDSESSION_ACTIVE)
			{
				CareerBackendData playerProfile = new CareerBackendData();
				m_mPlayerProfiles.Set(playerID, playerProfile);
				playerProfile = GetPlayerProfile(playerID);
				
				if (m_Callback)
					GetGame().GetBackendApi().PlayerData(playerProfile, playerID);
				
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
	//!
	//! \param[in] playerID
	void LoadConnectingPlayerProfile(int playerID)
	{
		if (!LoadPlayerProfileFromBackend(playerID))
		{
			m_aPlayerIDsToLoadProfile.Insert(playerID);
		}
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
		owner.SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_fCurrentRefreshTime = m_fRefreshTime;
		m_mPlayerProfiles = new map<int, ref CareerBackendData>();
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(owner.FindComponent(SCR_RespawnSystemComponent));
		
		if (!respawnSystem)
		{
			Print("There is no RespawnSystemComponent attached to the GameMode entity. Faction scoring will not work.", LogLevel.WARNING);
			return;
		}
		
		s_RespawnSystemComponent = respawnSystem;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_PlayerProfileManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
