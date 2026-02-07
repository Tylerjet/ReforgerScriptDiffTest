[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Handles respawn timers for players.")]
class SCR_WaveRespawnTimerComponentClass: SCR_RespawnTimerComponentClass
{
};

#define RESPAWN_TIMER_COMPONENT_DEBUG

//------------------------------------------------------------------------------------------------
//! Must be attached to a GameMode
// TODO@AS: Revisit after changes in timer comp
class SCR_WaveRespawnTimerComponent : SCR_RespawnTimerComponent
{	
	//! Respawn timers per faction
	[RplProp()]
	protected ref array<ref SCR_RespawnTimer> m_aFactionRespawnTimers = new ref array<ref SCR_RespawnTimer>();

	[RplProp()]
	protected ref array<int> m_aAllowedPlayers = new array<int>();

	[RplProp()]
	protected ref array<int> m_aWaitingPlayers = new array<int>();
	
	//! Mandatory component
	protected SCR_RespawnSystemComponent m_pRespawnSystemComponent;

	#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
	override void DrawDebugInfo()
	{
		super.DrawDebugInfo();

		DbgUI.Begin("Wave Respawn Diag");
		{
			// Players ready to respawn
			DbgUI.Text("Allowed Players:");
			PlayerManager playerManager = GetGame().GetPlayerManager();
			foreach (int playerId : m_aAllowedPlayers)
			{
				string playerText = string.Format("%1: %2", playerId, playerManager.GetPlayerName(playerId));
				DbgUI.Text(playerText);
			}

			// Players waiting in wave
			DbgUI.Text("Waiting Players:");
			foreach (int playerId : m_aWaitingPlayers)
			{
				string playerText = string.Format("%1: %2", playerId, playerManager.GetPlayerName(playerId));
				DbgUI.Text(playerText);
			}

			float timeNow = GetCurrentTime();
			if (m_pRespawnSystemComponent)
			{
				// Respawn timers
				DbgUI.Text("Faction Timers:");
				int index = 0;
				foreach (SCR_RespawnTimer respawnTimer : m_aFactionRespawnTimers)
				{
					Faction affiliatedFaction = m_pRespawnSystemComponent.GetFactionByIndex( index );
					if (affiliatedFaction)
					{
						string factionText = string.Format("%1: %2s (%3)", index, respawnTimer.GetRemainingTime(timeNow), affiliatedFaction.GetFactionName());
						DbgUI.Text(factionText);
					}

					index++;
				}
			}

		}
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override bool GetCanPlayerSpawn(int playerID)
	{
		return m_aAllowedPlayers.Contains(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerRemainingTime(int playerID)
	{
		// Ready to respawn
		if (m_aAllowedPlayers.Contains(playerID))
			return 0;

		// Waiting in next wave
		Faction faction = m_pRespawnSystemComponent.GetPlayerFaction(playerID);
		int factionIndex = m_pRespawnSystemComponent.GetFactionIndex(faction);
		if (factionIndex < 0)
			return 0;

		float timeNow = GetCurrentTime();
		return m_aFactionRespawnTimers[factionIndex].GetRemainingTime(timeNow);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		int allowedIndex = m_aAllowedPlayers.Find(playerId);
		if (allowedIndex != -1)
		{
			m_aAllowedPlayers.Remove( allowedIndex );
		}

		int waitingIndex = m_aWaitingPlayers.Find(playerId);
		if (waitingIndex != -1)
		{
			m_aWaitingPlayers.Remove( waitingIndex );
		}

		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killer Entity of killer instigator if any.
	*/
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		if (!m_aWaitingPlayers.Contains(playerId))
		{
			m_aWaitingPlayers.Insert(playerId);
			Replication.BumpMe();
		}
	}
	override void OnPlayerDeleted(int playerId, IEntity player)
	{
		OnPlayerKilled(playerId, player, null);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (!m_aWaitingPlayers.Contains(playerId))
		{
			m_aWaitingPlayers.Insert(playerId);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Broadcasts that sets timer duration for provided faction.
		\param duration Duration of respawn timer.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_StartFactionTimer(int factionIndex, float rplTime)
	{
		SCR_RespawnTimer timer = m_aFactionRespawnTimers[factionIndex];
		timer.Start(rplTime);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!IsMaster())
			return;
				
		bool shouldBump;
		float timeNow = GetCurrentTime();
		// Update timers
		int timersCount = m_aFactionRespawnTimers.Count();
		for (int i = 0; i < timersCount; i++)
		{
			SCR_RespawnTimer timer = m_aFactionRespawnTimers[i];
			if (timer)
			{
				// If timer is finished, allow spawn for current wave of players
				if (timer.IsFinished(timeNow))
				{
					// Add each waiting player of given faction into the "allow players" list
					for (int playerIndex = m_aWaitingPlayers.Count() -1; playerIndex >= 0; playerIndex--)
					{
						int playerId = m_aWaitingPlayers[playerIndex];
						Faction playerFaction = m_pRespawnSystemComponent.GetPlayerFaction(playerId);
						if (playerFaction)
						{
							int factionIndex = m_pRespawnSystemComponent.GetFactionIndex(playerFaction);
							if (factionIndex == i)
							{
								// It would be nicer to use set<T>, but rpl codec is missing for that object
								if (!m_aAllowedPlayers.Contains(playerId))
									m_aAllowedPlayers.Insert(playerId);

								// Remove from waiting list, we are in ready list already
								m_aWaitingPlayers.Remove(playerIndex);
								
								// Make sure to propagate changes
								shouldBump = true;
							}
						}
					}
					
					// Start timer
					RpcDo_StartFactionTimer(i, timeNow);
					Rpc(RpcDo_StartFactionTimer, i, timeNow);
				}
			}
		}
		
		if (shouldBump)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Initialize this component.
	*/
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		
		// Fetch neccessary components
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_pRespawnSystemComponent = SCR_RespawnSystemComponent.Cast(owner.FindComponent(SCR_RespawnSystemComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Initialize this component with data from FactionsManager.
	*/
	override void EOnInit(IEntity owner)
	{
		
		// Fill list of respawns with default data
		int factionsCount = 0;
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		if (factionManager)
			factionsCount = factionManager.GetFactionsList(factions);

		// Prefill list with data
		float timeNow = GetCurrentTime();
		for (int i = 0; i < factionsCount; i++)
		{
			ref SCR_RespawnTimer timer = new SCR_RespawnTimer();
			timer.SetDuration( m_fRespawnTime );
			timer.Start( timeNow - m_fRespawnTime );
			m_aFactionRespawnTimers.Insert( timer );
		}

		Replication.BumpMe();
	}
};
