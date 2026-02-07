[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of player penalties, kicks, bans etc.", color: "0 0 255 255")]
class SCR_PlayerPenaltyComponentClass: SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerPenaltyComponent: SCR_BaseGameModeComponent
{
	[Attribute("0", desc: "Penalty score for killing a friendly player.")]
	protected int m_iFriendlyPlayerKillPenalty;
	
	[Attribute("0", desc: "Penalty score for killing a friendly AI.")]
	protected int m_iFriendlyAIKillPenalty;
	
	[Attribute("0", desc: "Penalty score limit for a kick from the match.")]
	protected int m_iKickPenaltyLimit;
	
	[Attribute("0", desc: "Ban duration after a kick (in seconds, -1 for a session-long ban).")]
	protected int m_iBanDuration;
	
	[Attribute("0", desc: "How often penalty score substraction happens (in seconds).")]
	protected int m_iPenaltySubstractionPeriod;
	
	[Attribute("0", desc: "How many penalty points get substracted after each substraction period.")]
	protected int m_iPenaltySubstractionPoints;
	
	protected static SCR_PlayerPenaltyComponent s_Instance;
	protected static const int EVALUATION_PERIOD = 1000;
	
	protected RplComponent m_RplComponent;
	protected ref array<ref SCR_PlayerPenaltyData> m_aPlayerPenaltyData = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (IsProxy())
			return;
		
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (playerPenaltyData)
			playerPenaltyData.SetWasKicked(false);
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
			killerChar = GetInstigatorFromVehicle(instigator)
		}
		else
		{
			// Check if the killer is a regular soldier on foot
			killerChar = SCR_ChimeraCharacter.Cast(instigator);
			
			// If all else fails, check if the killer is in a vehicle turret
			if (!killerChar)
				killerChar = GetInstigatorFromVehicle(instigator, true)
		}
		
		if (!killerChar || entity == killerChar)
			return;
		
		int killerPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(killerChar);
		
		if (killerPlayerId == 0)
			return;
		
		if (!killerChar.GetFaction().IsFactionFriendly(victimChar.GetFaction()))
			return;
		
		int victimPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(victimChar);
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(killerPlayerId);
		
		if (!playerPenaltyData)
			return;
		
		if (victimPlayerId == 0)
			playerPenaltyData.AddPenaltyScore(m_iFriendlyAIKillPenalty);
		else
			playerPenaltyData.AddPenaltyScore(m_iFriendlyPlayerKillPenalty);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerPenaltyComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ChimeraCharacter GetInstigatorFromVehicle(IEntity veh, bool gunner = false)
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
	protected SCR_PlayerPenaltyData GetPlayerPenaltyData(int playerId)
	{
		if (IsProxy())
			return null;
		
		SCR_PlayerPenaltyData playerPenaltyData;

		// Check if the client is reconnecting
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
		playerPenaltyData = new SCR_PlayerPenaltyData;
		playerPenaltyData.SetPlayerId(playerId);
		m_aPlayerPenaltyData.Insert(playerPenaltyData);
		
		return playerPenaltyData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluatePlayerPenalties()
	{
		for (int i = 0, cnt = m_aPlayerPenaltyData.Count(); i < cnt; i++)
		{
			SCR_PlayerPenaltyData playerPenaltyData = m_aPlayerPenaltyData[i];
			
			// Periodically forgive a portion of penalty score, don't go below zero
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubstractionTimestamp() < Replication.Time())
			{
				int forgivenScore;
				
				if (m_iPenaltySubstractionPoints > playerPenaltyData.GetPenaltyScore())
					forgivenScore = playerPenaltyData.GetPenaltyScore();
				else
					forgivenScore = m_iPenaltySubstractionPoints;
				
				playerPenaltyData.AddPenaltyScore(-forgivenScore);
			}
			
			int playerId = playerPenaltyData.GetPlayerId();
			
			// Player is not connected
			if (!GetGame().GetPlayerManager().GetPlayerController(playerId))
				continue;
			
			// Player is host
			if (playerId == SCR_PlayerController.GetLocalPlayerId())
				continue;
			
			float bannedUntil = playerPenaltyData.GetBannedUntil();
			
			// If a player reconnected within ban duration, kick them immediately
			if (bannedUntil > Replication.Time())
			{
				KickPlayer(playerId, playerPenaltyData.GetKickReason());
				continue;
			}
			
			// Check penalty limit for kick / ban
			if (m_iKickPenaltyLimit > 0 && playerPenaltyData.GetPenaltyScore() >= m_iKickPenaltyLimit)
			{
				BanPlayer(playerId, m_iBanDuration, SCR_PlayerManagerKickReason.FRIENDLY_FIRE);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPenaltyScore(int playerId, int points)
	{
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (!playerPenaltyData)
			return;
		
		playerPenaltyData.AddPenaltyScore(points);
	}
	
	//------------------------------------------------------------------------------------------------
	void KickPlayer(int playerId, SCR_PlayerManagerKickReason reason, bool showNotification = true)
	{
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (playerPenaltyData)
		{
			playerPenaltyData.SetWasKicked(true);
			playerPenaltyData.SetKickReason(reason);
		}
		
		GetGame().GetPlayerManager().KickPlayer(playerId, reason);
		
		if (showNotification)
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KICKED, playerId, reason);
	}
	
	//------------------------------------------------------------------------------------------------
	void BanPlayer(int playerId, int duration, SCR_PlayerManagerKickReason reason)
	{
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (!playerPenaltyData)
			return;
		
		// Refresh ban timer only for players not yet banned
		if (playerPenaltyData.GetBannedUntil() < Replication.Time() && playerPenaltyData.GetBannedUntil() >= 0)
		{
			if (duration < 0)
				playerPenaltyData.SetBannedUntil(-1);
			else
				playerPenaltyData.SetBannedUntil(Replication.Time() + (duration * 1000));	// Converting s to ms
		}
		
		// Don't kick the player again
		if (playerPenaltyData.GetWasKicked())
			return;
		
		if (duration < 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED_NO_DURATION, playerId);
		else 
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED, playerId, duration);
		
		KickPlayer(playerId, reason, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbanPlayer(int playerId)
	{
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (!playerPenaltyData)
			return;
		
		playerPenaltyData.SetBannedUntil(0);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPenaltySubstractionPeriod()
	{
		return m_iPenaltySubstractionPeriod * 1000;	// Converting s to ms
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRemainingBanDuration(int playerId)
	{
		SCR_PlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (!playerPenaltyData)
			return 0;
		
		float bannedUntil = playerPenaltyData.GetBannedUntil();
		
		if (bannedUntil == -1)
			return -1;
		
		float banDuration = bannedUntil - Replication.Time();
		banDuration = Math.Max(0, banDuration);
		
		return banDuration / 1000;	// Converting ms to ms
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
		
		if (m_iBanDuration < m_iPenaltySubstractionPeriod)
			Print("SCR_PlayerPenaltyComponent: Ban duration is shorter than Penalty substraction period. This will cause the player to remain banned until their penalty is substracted.", LogLevel.WARNING);
		
		if (!GetGame().InPlayMode())
			return;
		
		s_Instance = this;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		GetGame().GetCallqueue().CallLater(EvaluatePlayerPenalties, EVALUATION_PERIOD, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerPenaltyComponent()
	{
		if (m_aPlayerPenaltyData)
		{
			m_aPlayerPenaltyData.Clear();
			m_aPlayerPenaltyData = null;
		}
	}
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerPenaltyData
{
	protected int m_iPlayerId;
	protected float m_fBannedUntil;
	protected int m_iPenaltyScore;
	protected float m_fNextPenaltySubstractionTimestamp;
	protected bool m_bWasKicked;
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
	void SetBannedUntil(float timestamp)
	{
		m_fBannedUntil = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBannedUntil()
	{
		return m_fBannedUntil;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPenaltyScore(int points)
	{
		m_iPenaltyScore += points;
		
		// Start the timer on penalty substraction when player was penalized while the timer was stopped
		if ((points > 0 && m_fNextPenaltySubstractionTimestamp < Replication.Time()) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubstractionTimestamp = Replication.Time() + SCR_PlayerPenaltyComponent.GetInstance().GetPenaltySubstractionPeriod();
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPenaltyScore()
	{
		return m_iPenaltyScore;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNextPenaltySubstractionTimestamp(float timestamp)
	{
		m_fNextPenaltySubstractionTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetNextPenaltySubstractionTimestamp()
	{
		return m_fNextPenaltySubstractionTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWasKicked(bool kicked)
	{
		m_bWasKicked = kicked;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetWasKicked()
	{
		return m_bWasKicked;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetKickReason(SCR_PlayerManagerKickReason reason)
	{
		m_eKickReason = reason;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_PlayerManagerKickReason GetKickReason()
	{
		return m_eKickReason;
	}
};