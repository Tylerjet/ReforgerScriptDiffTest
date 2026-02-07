[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Handles respawn timers for players.")]
class SCR_WaveRespawnTimerComponentClass : SCR_RespawnTimerComponentClass
{
}

#define RESPAWN_TIMER_COMPONENT_DEBUG

//! Must be attached to a GameMode
// TODO@AS: Revisit after changes in timer comp
class SCR_WaveRespawnTimerComponent : SCR_RespawnTimerComponent
{	
	//! Respawn timers per faction
	[RplProp()]
	protected ref array<ref SCR_RespawnTimer> m_aFactionRespawnTimers = {};

	[RplProp()]
	protected ref array<int> m_aAllowedPlayers = {};

	[RplProp()]
	protected ref array<int> m_aWaitingPlayers = {};
	
	#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
	//------------------------------------------------------------------------------------------------
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

			WorldTimestamp timeNow = GetCurrentTime();
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			
			if (factionManager)
			{
				// Respawn timers
				DbgUI.Text("Faction Timers:");
				int index = 0;
				foreach (SCR_RespawnTimer respawnTimer : m_aFactionRespawnTimers)
				{
					Faction affiliatedFaction = factionManager.GetFactionByIndex(index);
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
	override bool GetCanPlayerSpawn(int playerID, float additionalTime = 0)
	{
		return m_aAllowedPlayers.Contains(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerRemainingTime(int playerID, float additionalTime = 0)
	{
		// Ready to respawn
		if (m_aAllowedPlayers.Contains(playerID))
			return 0;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;

		// Waiting in next wave
		Faction faction = factionManager.GetPlayerFaction(playerID);
		int factionIndex = factionManager.GetFactionIndex(faction);
		if (factionIndex < 0)
			return 0;

		WorldTimestamp timeNow = GetCurrentTime();
		return m_aFactionRespawnTimers[factionIndex].GetRemainingTime(timeNow);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		int allowedIndex = m_aAllowedPlayers.Find(playerId);
		if (allowedIndex != -1)
			m_aAllowedPlayers.Remove( allowedIndex );

		int waitingIndex = m_aWaitingPlayers.Find(playerId);
		if (waitingIndex != -1)
			m_aWaitingPlayers.Remove( waitingIndex );

		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called after a player gets killed.
	//! \param playerId PlayerId of victim player.
	//! \param player Entity of victim player if any.
	//! \param killerEntity Entity of killer instigator if any.
	//! \param killer Instigator of the kill
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		if (!m_aWaitingPlayers.Contains(playerId))
		{
			m_aWaitingPlayers.Insert(playerId);
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDeleted(int playerId, IEntity player)
	{
		OnPlayerKilled(playerId, player, null, Instigator.CreateInstigator(null));
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
	//! Broadcasts that sets timer duration for provided faction.
	//! \param duration Duration of respawn timer.
	//! \param[in] factionIndex
	//! \param[in] rplTime
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_StartFactionTimer(int factionIndex, WorldTimestamp rplTime)
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
		WorldTimestamp timeNow = GetCurrentTime();
		// Update timers
		int timersCount = m_aFactionRespawnTimers.Count();
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction playerFaction;
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
						playerFaction = factionManager.GetPlayerFaction(playerId);
						if (playerFaction)
						{
							int factionIndex = factionManager.GetFactionIndex(playerFaction);
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
	//! Initialise this component.
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		
		// Fetch necessary components
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialise this component with data from FactionsManager.
	override void EOnInit(IEntity owner)
	{
		// Fill list of respawns with default data
		int factionsCount;
		array<Faction> factions = {};
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionsCount = factionManager.GetFactionsList(factions);

		// Prefill list with data
		SCR_RespawnTimer timer;
		WorldTimestamp timeNow = GetCurrentTime();
		for (int i = 0; i < factionsCount; i++)
		{
			timer = new SCR_RespawnTimer();
			timer.SetDuration(m_fRespawnTime);
			timer.Start(timeNow.PlusSeconds(-m_fRespawnTime));
			m_aFactionRespawnTimers.Insert(timer);
		}

		Replication.BumpMe();
	}
}
