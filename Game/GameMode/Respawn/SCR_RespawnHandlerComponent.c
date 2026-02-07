//------------------------------------------------------------------------------------------------
class SCR_RespawnHandlerComponentClass: SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
/*
	Component responsible for handling spawn logic.
	Must be attached to a GameMode. Can be derived to implement custom logic,
	like automatic re-spawning or spawning via menu.
*/
class SCR_RespawnHandlerComponent : SCR_BaseGameModeComponent
{

	/*!
		Set of unique players that are enqueued/eligible for respawning.
		This means that they might not be able to respawn due to custom game mode conditions,
		but they should already check whether they can respawn and process such requests.

		Only relevant to the server.
	*/
	protected ref set<int> m_sEnqueuedPlayers = new set<int>();

	/*!
		Returns whether given player by their playerId is enqueued for respawning.
	*/
	bool IsPlayerEnqueued(int playerId)
	{
		return m_sEnqueuedPlayers.Contains(playerId);
	}

	/*!
		Returns whether given player by their playerId can respawn.

		This can be used to handle necessary selection dependencies, etc.
		\return True in case player can spawn, false otherwise.
	*/
	bool CanPlayerSpawn(int playerId)
	{
		SCR_RespawnSystemComponent respawnSystem = m_pGameMode.GetRespawnSystemComponent();

		if (!respawnSystem.GetPlayerLoadout(playerId))
			return false;

		if (!respawnSystem.GetPlayerSpawnPoint(playerId))
			return false;

		return true;
	}

	/*
		Enqueue player to be eligible to respawn, see m_sEnqueuedPlayers.
		Only relevant to the server.

		\param PlayerId of enqueued player.
	*/
	protected void EnqueuePlayer(int playerId)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		if (m_sEnqueuedPlayers.Insert(playerId))
		{
			OnPlayerEnqueued(playerId);

			SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
			respawnComponent.AcknowledgePlayerEnqueued();
		}
	}

	/*!
		Raised when a player is successfully enqueued for respawning.
		Called on the server only, passed player can be any client.
		\param playerId PlayerId of enqueued player.
	*/
	protected void OnPlayerEnqueued(int playerId)
	{
	}

	/*!
		Raised when a player is successfully enqueued for respawning.
		Called only when the enqueued player is local machine.
	*/
	void OnLocalPlayerEnqueued()
	{
		#ifdef ENABLE_DIAG
		if (IsCLISpawnEnabled())
		{
			if (UseCLISpawn())
				GetGame().GetPlayerController().RequestRespawn();
		}
		#endif
	}

	/*!
		Dequeue player from eligibility to respawn, see m_sEnqueuedPlayers.
		Only relevant to the server.

		\param PlayerId of dequeued player.
	*/
	protected bool DequeuePlayer(int playerId)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return false;

		int index = m_sEnqueuedPlayers.Find(playerId);
		if (index == -1)
			return false;

		m_sEnqueuedPlayers.Remove(index);
		OnPlayerDequeued(playerId);

		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		respawnComponent.AcknowledgePlayerDequeued();

		return true;
	}

	/*!
		Raised when a player is successfully dequeued from respawning.
		Called on the server only, passed player can be any client.

		\param playerId PlayerId of enqueued player.
	*/
	protected void OnPlayerDequeued(int playerId)
	{
	}

	/*!
		Raised when a player is sucessfully dequeued from respawning.
		Called only when the dequeued player is local machine.
	*/
	void OnLocalPlayerDequeued()
	{
	}

	/*!
		Called on every machine after a player is registered (identity, name etc.). Always called after OnPlayerConnected.
		\param playerId PlayerId of registered player.
	*/
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		// If, for any reason valid or invalid, the target player is already
		// enqueued on their registranion, dequeue them, so they can be
		// enqueued and NOTIFIED properly
		if (m_sEnqueuedPlayers.Contains(playerId))
			DequeuePlayer(playerId);

		EnqueuePlayer(playerId);
	}

	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killer Entity of killer instigator if any.
	*/
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);

		EnqueuePlayer(playerId);
	}
	override void OnPlayerDeleted(int playerId, IEntity player)
	{
		OnPlayerKilled(playerId, player, null);
	}

	/*!
		Called after a player is spawned.
		\param playerId PlayerId of spawned player.
		\param controlledEntity Spawned entity for this player.
	*/
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		DequeuePlayer(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player is disconnected.
		\param playerId PlayerId of disconnected player.
	*/
	override void OnPlayerDisconnected(int playerId)
	{
		super.OnPlayerDisconnected(playerId);

		// This player is no longer eligible for spawn, as they are inactive.
		DequeuePlayer(playerId);
	}

	/*!
		Randomize spawn point for given player - server-only.
		\param PlayerId PlayerId of player to randomize spawn point for.
	*/
	protected bool RandomizePlayerSpawnPoint(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return false;

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();


		SCR_SpawnPoint spawnPoint;
		// Choose faction spawn point
		Faction playerFaction = respawnSystem.GetPlayerFaction(playerId);
		if (playerFaction)
		{
			FactionKey factionKey = playerFaction.GetFactionKey();
			spawnPoint = SCR_SpawnPoint.GetRandomSpawnPointForFaction(factionKey);
		}
		// Choose any spawn point
		else
		{
			spawnPoint = SCR_SpawnPoint.GetRandomSpawnPointDeathmatch();
		}

		if (!spawnPoint)
		{
			Print("No spawn point could be picked, are there any present in the world?", LogLevel.ERROR);
			return false;
		}

		RplId spawnPointRplId = SCR_SpawnPoint.GetSpawnPointRplId(spawnPoint);
		if (!spawnPointRplId.IsValid())
		{
			Print("Picked spawn point has no RplId, is it replicated?", LogLevel.ERROR);
			return false;
		}

		respawnSystem.DoSetPlayerSpawnPoint(playerId, spawnPointRplId);
		return true;
	}

	/*!
		Randomize faction for given player - server-only.
		\param PlayerId PlayerId of player to randomize faction for.
	*/
	protected bool RandomizePlayerFaction(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return false;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return false;

		// Fetch all factions
		array<Faction> factions = {};
		int factionsCount = factionManager.GetFactionsList(factions);
		if (factionsCount == 0)
		{
			Print("Could not randomize player faction, no factions are available!", LogLevel.ERROR);
			return false;
		}

		// Filter available factions only
		array<Faction> availableFactions = {};
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		foreach (Faction faction : factions)
		{
			if (respawnSystem.CanSetFaction(playerId, respawnSystem.GetFactionIndex(faction)))
				availableFactions.Insert(faction);
		}

		// Get proper faction index
		int randomFactionIndex = Math.RandomInt(0, availableFactions.Count());
		int factionIndex = respawnSystem.GetFactionIndex(availableFactions[randomFactionIndex]);

		SCR_RespawnSystemComponent.GetInstance().DoSetPlayerFaction(playerId, factionIndex);
		return true;
	}

	/*
		Randomize loadout for given player - server-only.
		\param PlayerId PlayerId of player to randomize loadout for.
	*/
	protected bool RandomizePlayerLoadout(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return false;

		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (!loadoutManager)
			return false;

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		Faction playerFaction = respawnSystem.GetPlayerFaction(playerId);
		int loadoutIndex;

		// Pick random faction based loadout
		if (playerFaction)
		{
			SCR_BasePlayerLoadout randomLoadout = loadoutManager.GetRandomFactionLoadout(playerFaction);
			loadoutIndex = loadoutManager.GetLoadoutIndex(randomLoadout);
		}
		// Pick completely random
		else
		{
			loadoutIndex = loadoutManager.GetRandomLoadoutIndex();
		}

		SCR_RespawnSystemComponent.GetInstance().DoSetPlayerLoadout(playerId, loadoutIndex);
		return true;
	}

	/*
		Randomize group for given player - server-only.
		\param PlayerId PlayerId of player to randomize group for.
	*/
	protected bool RandomizePlayerGroup(int playerId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerId);
		Faction playerFaction = SCR_RespawnSystemComponent.GetInstance().GetPlayerFaction(playerId);
		if (!playerFaction)
			return false;
		if (groupsManager && playerGroupController && !groupsManager.IsPlayerInAnyGroup(playerId))
		{
			SCR_AIGroup group = groupsManager.GetFirstNotFullForFaction(playerFaction, null, true);
			if (!group)
				playerGroupController.RequestCreateGroup();
			else
				playerGroupController.RequestJoinGroup(group.GetGroupID());
		}

		return true;
	}

	/*!
		Ticks every frame. Handle enqueued players as necessary.
	*/
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
	}

	/*!
		Initialize this component.
	*/
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}

	/*!
		Initialize this component so init and frame event(s) are thrown.
	*/
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	#ifdef ENABLE_DIAG
	static bool IsCLISpawnEnabled() 
	{
		return System.IsCLIParam("autodeployFaction") || System.IsCLIParam("autodeployLoadout") || 
		 	System.IsCLIParam("tdmf") || System.IsCLIParam("tdml");
	}
	//------------------------------------------------------------------------------------------------
	static bool UseCLISpawn()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager == null)
			return false;
		
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (loadoutManager == null)
			return false;
		
		int factionId = -1;
		string fs;
		if (System.GetCLIParam("autodeployFaction", fs) || System.GetCLIParam("tdmf", fs))
		{
			Faction factionFromKey = factionManager.GetFactionByKey(fs);
			
			if (factionFromKey != null)
				factionId = factionManager.GetFactionIndex(factionFromKey);
			else
				factionId = fs.ToInt();
		}
			
		int loadoutId = -1;
		string ls;
		if (System.GetCLIParam("autodeployLoadout", fs) || System.GetCLIParam("tdml", ls))
		{
			SCR_BasePlayerLoadout loadoutFromKey = loadoutManager.GetLoadoutByName(ls);
			
			if (loadoutFromKey != null)
				loadoutId = loadoutManager.GetLoadoutIndex(loadoutFromKey);
			else
				loadoutId = ls.ToInt();	
		}
		
		Faction faction = factionManager.GetFactionByIndex(factionId);
		SCR_BasePlayerLoadout loadout;

		/// Resolve faction -> loadout first, this is should be the most relevant case,
		/// where user provided
		if (faction)
		{
			SCR_BasePlayerLoadout loadoutFromIndex = loadoutManager.GetLoadoutByIndex(loadoutId);
			SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(loadoutFromIndex);
			if (factionLoadout && factionLoadout.GetFactionKey() != faction.GetFactionKey())
			{
				Print("Cannot request spawn with loadout: " + loadoutId + " because it does not belong to faction: " + factionId + "!", LogLevel.ERROR);
				return false;
			}

			/// When only the faction is provided, we will just select random loadout
			if (!loadoutFromIndex)
			{
				array<ref SCR_BasePlayerLoadout> loadouts = {};
				loadoutManager.GetPlayerLoadoutsByFaction(faction, loadouts);
				loadout = loadouts.GetRandomElement();
			}
			else
				loadout = loadoutFromIndex;
		}

		/// Resolve loadout -> faction second
		if (!faction && loadoutId >= 0)
		{
			SCR_BasePlayerLoadout loadoutByIndex = loadoutManager.GetLoadoutByIndex(loadoutId);
			if (loadoutByIndex)
			{
				loadout = loadoutByIndex;
				SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(loadoutByIndex);
				if (factionLoadout)
					faction = factionManager.GetFactionByKey(factionLoadout.GetFactionKey());
			}
			else
			{
				Print("Cannot request spawn with loadout: " + loadoutId + " because it is not valid!", LogLevel.ERROR);
				return false;
			}
		}

		/// Now we must make sure that our data is valid and select adequate spawn point
		if (!faction || !loadout)
		{
			Print("Cannot request spawn with loadout: " + loadoutId + " and faction: " + factionId + ", these do not seem to be valid!", LogLevel.ERROR);
			return false;
		}

		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey());
		if (spawnPoints.IsEmpty())
		{
			Print(string.Format("Cannot request spawn with loadout: %1 and faction: %2 because no spawn points exist!", loadoutId, factionId), LogLevel.ERROR);
			return false;
		}
		
		SCR_SpawnPoint spawnPoint = spawnPoints.GetRandomElement();
		if (!spawnPoint)
		{
			Print("Cannot request spawn with loadout: " + loadoutId + " and faction: " + factionId + " because no valid spawn point exists!", LogLevel.ERROR);
			return false;
		}
		
		PlayerController localController = GetGame().GetPlayerController();
		if (!localController)
			return false;
		
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(localController.GetRespawnComponent());
		respawnComponent.RequestCLISpawnInfo(faction, loadout, spawnPoint);
		return true;
	}
	#endif
};