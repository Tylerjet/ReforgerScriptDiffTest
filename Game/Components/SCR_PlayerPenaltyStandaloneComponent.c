#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of player penalties, kicks, bans etc.", color: "0 0 255 255")]
class SCR_PlayerPenaltyStandaloneComponentClass: SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerPenaltyStandaloneComponent: SCR_BaseGameModeComponent
{
	[Attribute("3", desc: "Penalty score for killing a friendly player.")]
	protected int m_iFriendlyPlayerKillPenalty;
	
	[Attribute("1", desc: "Penalty score for killing a friendly AI.")]
	protected int m_iFriendlyAIKillPenalty;
	
	[Attribute("10", desc: "Penalty score limit for a kick from the match.")]
	protected int m_iKickPenaltyLimit;
	
	[Attribute("1800", desc: "Ban duration after a kick (in seconds, -1 for a session-long ban).")]
	protected int m_iBanDuration;
	
	[Attribute("900", desc: "How often penalty score subtraction happens (in seconds).")]
	protected int m_iPenaltySubtractionPeriod;
	
	[Attribute("2", desc: "How many penalty points get substracted after each subtraction period.")]
	protected int m_iPenaltySubtractionPoints;
	
	protected static SCR_PlayerPenaltyStandaloneComponent s_Instance;
	protected static const int EVALUATION_PERIOD = 1000;
	
	protected RplComponent m_RplComponent;
	protected ref array<ref SCR_PlayerPenaltyStandaloneData> m_aPlayerPenaltyData = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (IsProxy())
			return;
		
		//GetPlayerPenaltyData creates the PlayerPenaltyStandaloneData for this playerId if it doesn't exist
		GetPlayerPenaltyData(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
		if (IsProxy())
			return;
		
		if (m_iFriendlyAIKillPenalty == 0 && m_iFriendlyPlayerKillPenalty == 0)
			return;
		
		if (!instigator)
			return;
		
		if (entity == instigator)
			return;
		
		SCR_ChimeraCharacter victimChar = SCR_ChimeraCharacter.Cast(entity);
		
		if (!victimChar)
			return;
		
		SCR_ChimeraCharacter killerChar;
		
		// Instigator is a vehicle, find the driver
		if (instigator.IsInherited(Vehicle))
		{
			Print("ERROR: Instigator of this kill was a vehicle. This is not the expected behaviour!", LogLevel.ERROR);
			
			//That should not happen, but just to be safe for now, we have this way to calculate the correct instigator
			killerChar = GetInstigatorFromVehicle(instigator)
		}
		else
		{
			// Check if the killer is a regular soldier
			killerChar = SCR_ChimeraCharacter.Cast(instigator);
			
			// If all else fails, check if the killer is in a vehicle turret
			if (!killerChar)
			{
				Print("DEBUG LINE | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
				Print("ERROR: Instigator of this kill could not be casted to SCR_ChimeraCharacter", LogLevel.ERROR);
				killerChar = GetInstigatorFromVehicle(instigator, true);
			}
		}
		
		//If there's no killer, or its a suicide: return
		if (!killerChar || killerChar == victimChar)
			return;
		
		int killerPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(killerChar);
		
		if (killerPlayerId == 0)
			return;
		
		//If it's no friendly kill, no wrongdoing was committed
		if (!killerChar.GetFaction().IsFactionFriendly(victimChar.GetFaction()))
			return;
		
		int victimPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(victimChar);
		SCR_PlayerPenaltyStandaloneData playerPenaltyData = GetPlayerPenaltyData(killerPlayerId);
		
		if (victimPlayerId == 0)
			playerPenaltyData.AddPenaltyScore(m_iFriendlyAIKillPenalty);
		else
			playerPenaltyData.AddPenaltyScore(m_iFriendlyPlayerKillPenalty);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerPenaltyStandaloneComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//IT SHOULD NOT BE STATIC. TODO: IMPROVE THIS
	static SCR_ChimeraCharacter GetInstigatorFromVehicle(IEntity veh, bool gunner = false)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
			
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
		
		for (int i = 0, cnt = compartmentManager.GetCompartments(compartments); i < cnt; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			
			if ((!gunner && slot.Type() == PilotCompartmentSlot) || (gunner && slot.Type() == TurretCompartmentSlot))
				return SCR_ChimeraCharacter.Cast(slot.GetOccupant());
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_PlayerPenaltyStandaloneData GetPlayerPenaltyData(int playerId)
	{
		if (IsProxy() || playerId == 0)
			return null;
		
		SCR_PlayerPenaltyStandaloneData playerPenaltyData;

		// Check if the client is reconnecting
		//Also if there's going to be many entries, the search might prove very expensive.
		for (int i = 0, cnt = m_aPlayerPenaltyData.Count(); i < cnt; i++)
		{
			if (m_aPlayerPenaltyData[i].GetPlayerId() == playerId)
			{
				playerPenaltyData = m_aPlayerPenaltyData[i];
				break;
			}
		}
		
		// Client reconnected, return saved data
		if (playerPenaltyData)
			return playerPenaltyData;
		
		// Check validity of playerId before registering new data
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!pc)
		{
			Print(string.Format("SCR_PlayerPenaltyComponent: No player with playerId %1 found.", playerId), LogLevel.ERROR);
			return null;
		}
		
		// First connection, register new data
		playerPenaltyData = new SCR_PlayerPenaltyStandaloneData();
		playerPenaltyData.SetPlayerId(playerId);
		m_aPlayerPenaltyData.Insert(playerPenaltyData);
		
		return playerPenaltyData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluatePlayerPenalties()
	{
		for (int i = 0, cnt = m_aPlayerPenaltyData.Count(); i < cnt; i++)
		{
			SCR_PlayerPenaltyStandaloneData playerPenaltyData = m_aPlayerPenaltyData[i];
			
			// Periodically forgive a portion of penalty score, don't go below zero
			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubstractionTimestamp() < Replication.Time())
			#else
			ChimeraWorld world = GetOwner().GetWorld();
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubstractionTimestamp().Less(world.GetServerTimestamp()))
			#endif
			{
				int forgivenScore;
				
				if (m_iPenaltySubtractionPoints > playerPenaltyData.GetPenaltyScore())
					forgivenScore = playerPenaltyData.GetPenaltyScore();
				else
					forgivenScore = m_iPenaltySubtractionPoints;
				
				playerPenaltyData.AddPenaltyScore(-forgivenScore);
			}
			
			int playerId = playerPenaltyData.GetPlayerId();
			
			// Player is not connected
			if (!GetGame().GetPlayerManager().GetPlayerController(playerId))
				continue;
			
			// Player is host
			if (playerId == SCR_PlayerController.GetLocalPlayerId())
				continue;
			
			// Check penalty limit for kick / ban
			if (m_iKickPenaltyLimit > 0 && playerPenaltyData.GetPenaltyScore() >= m_iKickPenaltyLimit)
			{
				// TODO: Use callback from backend instead
				KickPlayer(playerId, m_iBanDuration, SCR_PlayerManagerKickReason.FRIENDLY_FIRE);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Is this unused?
	void AddPenaltyScore(int playerId, int points)
	{
		BackendApi beApi = GetGame().GetBackendApi();
		
		//Currently, only friendly fire is tracked in this component
		if (beApi && points > 0)
			beApi.PlayerBanEvent("Trolling", "FriendlyFire", points, playerId);
		
		GetPlayerPenaltyData(playerId).AddPenaltyScore(points);
	}
	
	//------------------------------------------------------------------------------------------------
	void KickPlayer(int playerId, int duration, SCR_PlayerManagerKickReason reason)
	{	
		SCR_PlayerPenaltyStandaloneData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (playerPenaltyData)
			playerPenaltyData.AddPenaltyScore(-playerPenaltyData.GetPenaltyScore());
		
		GetGame().GetPlayerManager().KickPlayer(playerId, reason, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPenaltySubstractionPeriod()
	{
		return m_iPenaltySubtractionPeriod * 1000;	// Converting s to ms
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (IsProxy())
			return;
		
		if (m_iBanDuration < m_iPenaltySubtractionPeriod)
			Print("SCR_PlayerPenaltyComponent: Ban duration is shorter than Penalty substraction period. This will cause the player to remain banned until their penalty is substracted.", LogLevel.WARNING);
		
		if (!GetGame().InPlayMode())
			return;
		
		s_Instance = this;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		GetGame().GetCallqueue().CallLater(EvaluatePlayerPenalties, EVALUATION_PERIOD, true);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerPenaltyStandaloneData
{
	protected int m_iPlayerId;
	protected int m_iPenaltyScore;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fNextPenaltySubstractionTimestamp;
	#else
	protected WorldTimestamp m_fNextPenaltySubstractionTimestamp;
	#endif
	protected SCR_PlayerManagerKickReason m_eKickReason = SCR_PlayerManagerKickReason.DISRUPTIVE_BEHAVIOUR;
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerId(int playerId)
	{
		m_iPlayerId = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPenaltyScore(int points)
	{
		m_iPenaltyScore += points;
		
		// Start the timer on penalty substraction when player was penalized while the timer was stopped
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if ((points > 0 && m_fNextPenaltySubstractionTimestamp < Replication.Time()) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubstractionTimestamp = Replication.Time() + SCR_PlayerPenaltyStandaloneComponent.GetInstance().GetPenaltySubstractionPeriod();
		#else
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if ((points > 0 && m_fNextPenaltySubstractionTimestamp.Less(currentTime)) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubstractionTimestamp = currentTime.PlusMilliseconds(SCR_PlayerPenaltyStandaloneComponent.GetInstance().GetPenaltySubstractionPeriod());
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPenaltyScore()
	{
		return m_iPenaltyScore;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetNextPenaltySubstractionTimestamp(float timestamp)
	#else
	void SetNextPenaltySubstractionTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fNextPenaltySubstractionTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetNextPenaltySubstractionTimestamp()
	#else
	WorldTimestamp GetNextPenaltySubstractionTimestamp()
	#endif
	{
		return m_fNextPenaltySubstractionTimestamp;
	}
};