[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Main component used for collecting player data.")]
class SCR_DataCollectorComponentClass : SCR_BaseGameModeComponentClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_DataCollectorComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref array<ref SCR_DataCollectorModule> m_aModules;
	
	/*
	//This is part of the rework I am making of PlayerData so it becomes a pure container
	SCR_DataCollectorModule m_StoreModule;
	
	//This is part of the improvements I am making of the WarCrimes and the ProportionalityPrinciple
	protected bool warCrimesModule = false;
	*/
	[Attribute("1", UIWidgets.CheckBox, desc: "Are War Crimes detection enabled?")]
	protected bool m_bWarCrimesEnabled;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "Is the proportionality principle enabled?")]
	protected bool m_bWarCrimesProportionalityPrincipleEnabled;
	
	protected ref map<int, ref SCR_PlayerData> m_mPlayerData = new map<int, ref SCR_PlayerData>();

	protected IEntity m_Owner;

	protected ref SCR_DataCollectorUI m_UiHandler;

	protected ref map<FactionKey, ref array<float>> m_mFactionScore = new map<FactionKey, ref array<float>>();

#ifdef ENABLE_DIAG
	protected bool m_bLocalEntityListening = false;
	protected int m_iInitializingTimer = 0;
	protected bool m_bVisualDisplay = false;
	
	
#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerID;
		PlayerController playerController;
		SCR_DataCollectorCommunicationComponent communicationComponent;

		/*
		Here we add to the faction the scores of all the players who haven't disconnected yet
		*/
		SCR_ChimeraCharacter playerChimera;
		Faction faction;

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			playerID = m_mPlayerData.GetKey(i);

			//We update the duration of the session here because it should not be connected to any module
			m_mPlayerData.Get(playerID).CalculateSessionDuration();

			playerChimera = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!playerChimera)
				continue;

			faction = playerChimera.GetFaction();

			if (!faction)
				continue;

			AddStatsToFaction(faction.GetFactionKey(), m_mPlayerData.Get(playerID).CalculateStatsDifference());
		}

		/* We replicate the faction stats now, so they can be found in the client's machine*/
		array<FactionKey> factionKeys = {};
		array<float> factionValues = {};
		int valuesSize = 0;

		foreach (FactionKey key, array<float> value : m_mFactionScore)
		{
			factionKeys.Insert(key);
			factionValues.InsertAll(value);
			if (valuesSize == 0)
				valuesSize = value.Count();
		}

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			playerID = m_mPlayerData.GetKey(i);
			playerController = playerManager.GetPlayerController(playerID);
			if (!playerController)
				continue;

			communicationComponent = SCR_DataCollectorCommunicationComponent.Cast(playerController.FindComponent(SCR_DataCollectorCommunicationComponent));
			if (!communicationComponent)
				continue;

			communicationComponent.SendData(m_mPlayerData.Get(playerID), factionKeys, factionValues, valuesSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_EDataStats of all players belonging to a faction added
	//! Some of those stats don't make sense as they are (ie. Rank, LevelOfExperience)
	//! Bear that in mind when using these
	array<float> GetFactionStats(FactionKey key)
	{
		return m_mFactionScore.Get(key);
	}

	//------------------------------------------------------------------------------------------------
	//! GetAllFactionStats can be used in weird user cases
	//! Most of the time GetFactionStats should be enough
	//! Also, I am returning the pointer to the original attribute, so technically, this is unsafe. May make safer (or remove) in the future.
	//! Use responsibly!
	map<FactionKey, ref array<float>> GetAllFactionStats()
	{
		return m_mFactionScore;
	}

	//------------------------------------------------------------------------------------------------
	void AddStatsToFaction(FactionKey key, notnull array<float> stats)
	{
		array<float> factionStats = m_mFactionScore.Get(key);
		//If the faction doesn't exist in our map yet, just create it and initialize it with these stats
		if (!factionStats)
		{
			m_mFactionScore.Insert(key, stats);
		}
		else
		{
			int statsCount = stats.Count();
			
			if (statsCount == 0 || factionStats.Count() != statsCount)
			{
				Print("ERROR WHEN ADDING FACTIONSTATS IN DATA COLLECTOR: Size of faction stats is different than expected size! Expected size was" + statsCount + " but real size was "+ factionStats.Count(), LogLevel.ERROR);
				return;
			}
			
			for (int i = 0; i < statsCount; i++)
			{
				factionStats[i] = factionStats[i] + stats[i];
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_DataCollectorModule FindModule(typename type)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			if (m_aModules[i].Type() == type)
				return m_aModules[i];
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMaster()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return (rplComponent && rplComponent.IsMaster());
	}

	//------------------------------------------------------------------------------------------------
	ref Managed GetPlayerDataStats(int playerID)
	{
		SCR_PlayerData playerData = GetPlayerData(playerID);
		//Now this instance of playerData can be deleted from the array
		//We need this callqueue because we can't remove the instance from this array before it's sent to TD through c++
		GetGame().GetCallqueue().CallLater(RemovePlayer, 500, false, playerID);

		return playerData.GetDataEventStats();
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePlayer(int playerID)
	{
		m_mPlayerData.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerData GetPlayerData(int playerID, bool createNew = true, bool requestFromBackend = true)
	{
		SCR_PlayerData playerData = m_mPlayerData.Get(playerID);
		if (!playerData && createNew)
		{
			playerData = new SCR_PlayerData(playerID, true, requestFromBackend);
			playerData.ActivateWarCrimesDetection(m_bWarCrimesEnabled, m_bWarCrimesProportionalityPrincipleEnabled);
			m_mPlayerData.Insert(playerID, playerData);
		}

		return playerData;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayers(out notnull array<int> outPlayers)
	{
		if (m_mPlayerData.IsEmpty())
			return 0;

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			outPlayers.Insert(m_mPlayerData.GetKey(i));
		}

		return m_mPlayerData.Count();
	}

	//------------------------------------------------------------------------------------------------
	//OnAuditSuccess is the moment when the player has not only connected, but also been authenticated
	protected override void OnPlayerAuditSuccess(int playerId)
	{
		//We create the player's PlayerData here
		GetPlayerData(playerId);

		//And then let the modules handle the newly connected player if they need to
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerAuditSuccess(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		SCR_PlayerData playerDisconnectedData = GetPlayerData(playerId);
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerDisconnected(playerId);
		}

		playerDisconnectedData.StoreProfile();

		/* ! Add stats to faction */
		// Here we add the stats of the individual player who desconnected to the faction
		// We only do that if the game is not in POSTGAME state, because if it is we already added this player's stats to the faction in the OnGameModeEnd method

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode.GetState() != SCR_EGameModeState.POSTGAME)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			SCR_ChimeraCharacter playerChimera = SCR_ChimeraCharacter.Cast(player);
			if (playerChimera)
			{
				Faction faction = playerChimera.GetFaction();

				if (faction)
					AddStatsToFaction(faction.GetFactionKey(), playerDisconnectedData.CalculateStatsDifference());
			}
		}
		/* ! DONE ADDING STATS TO THE FACTION */
		//We cannot remove this instance of data from the player collector because the event has not been sent yet to the Database for tracking purposes
		//m_mPlayerData.Remove(playerId);

		//As an alternative, in GetPlayerDataStats we put this instance to be removed after its used in C++
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerSpawned(playerId, controlledEntity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerKilled(playerId, player, killer);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	This method is a hack to process killings when the dead entity is an AI
	Because there's no "OnAiKilled" method
	*/
	/* TODO: Finish the method checking for vehicles as well and uncomment it
	protected override void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
		if (!entity || !instigator)
			return;
		
		//if (!SCR_ChimeraCharacter.Cast(entity) || !SCR_ChimeraCharacter.Cast(instigator))
			
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		//If playerId is not 0 it means this will be handled by the OnPlayerKilled event
		//so we don't need to do anything else
		if (playerId != 0)
			return;
		
		int killerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(instigator);
		OnPlayerKilled(playerId, entity, instigator);
	}
	*/

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void OnPlayerEntityChanged(IEntity from, IEntity to)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnControlledEntityChanged(from, to);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! To debug this component we need to listen to the OnEntityChanged invoker from the local player
	protected void ListenToLocalControllerEntityChanged()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			m_bLocalEntityListening = true;
			playerController.m_OnControlledEntityChanged.Insert(OnPlayerEntityChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	//Prototyping method. ENABLE_DIAG CLI or #define required
	protected void CreateStatVisualizations()
	{
		if (!m_UiHandler)
			m_UiHandler = new SCR_DataCollectorUI();

		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].CreateVisualization();
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_DataCollectorUI GetUIHandler()
	{
		return m_UiHandler;
	}
#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeStart()
	{
		StartDataCollectorSession();
	}

	//------------------------------------------------------------------------------------------------
	//! We call StartDataCollectorSession when the backend session is ready (OnGameModeStart)
	//! because that's the time when we can check whether the server has writing privileges
	//! If the server has no writing privileges, we don't bother tracking their performance
	void StartDataCollectorSession()
	{
#ifndef ENABLE_DIAG
		//These are all the comprobations to make sure that this server has writing rights. If not, there's no need for a data collector
		BackendApi ba = GetGame().GetBackendApi();

		if (!ba || !IsMaster())
			return;

		SessionStorage baStorage = ba.GetStorage();

		if (!baStorage)
			return;

		if (!baStorage.GetOnlineWritePrivilege())
		{
			Print("DataCollectorComponent: StartDataCollectorSession: This server has no writing privileges.", LogLevel.DEBUG);
			return;
		}
#endif

		if (!m_Owner)
		{
			Print("DataCollectorComponent: StartDataCollectorSession: m_Owner is null. Can't add the EntityEvent.FRAME flag thus data collector will not work properly.", LogLevel.ERROR);
		}
		else
		{
			SetEventMask(m_Owner, EntityEvent.FRAME);
			m_Owner.SetFlags(EntityFlags.ACTIVE, false);
		}

		//Now we check if there is any player already to create playerData manually
		if (GetGame().GetPlayerManager().GetPlayerCount() <= 0)
			return;

		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			GetPlayerData(playerId);
		}

		SetEventMask(m_Owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
#ifdef ENABLE_DIAG
		//I am doing this because the OnAudit invoker is only used on server-side
		//Here we want to be able to debug the tracking of career stuff on client-side
		//since there's no event for it, we need to check periodically until
		//a local player controller is found
		if (!m_bLocalEntityListening)
		{
			if (m_iInitializingTimer % 100)
			{
				ListenToLocalControllerEntityChanged();
			}
			m_iInitializingTimer++;
		}

		
		if (m_bVisualDisplay != DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION_ENABLE_DIAG))
		{
			if (!m_UiHandler)
			{
				CreateStatVisualizations();
			}
			
			m_bVisualDisplay = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION_ENABLE_DIAG);
			m_UiHandler.SetVisible(m_bVisualDisplay);
		}
#endif

		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].Update(owner, timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		//If there is a data collector instance already, return
		if (GetGame().GetDataCollector())
			return;

		//Register the data collector
		GetGame().RegisterDataCollector(this);

		m_Owner = owner;
		
		//warCrimesModule = FindModule(SCR_DataCollectorWarCrimesModule);
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_DataCollectorComponent()
	{}
};
