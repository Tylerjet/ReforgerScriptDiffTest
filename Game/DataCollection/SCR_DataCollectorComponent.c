//#define DEBUG_CAREER
//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Main component used for collecting player data.")]
class SCR_DataCollectorComponentClass : ScriptComponentClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_DataCollectorComponent : ScriptComponent
{
	[Attribute()]
	protected ref array<ref SCR_DataCollectorModule> m_aModules;
	
	protected ref map<int, ref SCR_PlayerData> m_mPlayerData = new map<int, ref SCR_PlayerData>();
	
	protected SCR_DataCollectorUI m_UiComponent;
	
	protected IEntity m_Owner;
	
	//------------------------------------------------------------------------------------------------
	protected void ReplicatePlayerData()
	{
		#ifdef DEBUG_CAREER
			return;
		#endif
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerID;
		PlayerController playerController;
		SCR_DataCollectorCommunicationComponent communicationComponent;
		
		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			playerID = m_mPlayerData.GetKey(i);
			playerController = playerManager.GetPlayerController(playerID);
			if (!playerController)
				continue;
			
			communicationComponent = SCR_DataCollectorCommunicationComponent.Cast(playerController.FindComponent(SCR_DataCollectorCommunicationComponent));
			if (!communicationComponent)
				continue;
			
			communicationComponent.SendData(m_mPlayerData.Get(playerID));
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
	SCR_PlayerData GetPlayerData(int playerID, bool createNew = true, bool requestFromBackend = true)
	{
		SCR_PlayerData playerData = m_mPlayerData.Get(playerID);
		if (!playerData && createNew)
		{
			playerData = new SCR_PlayerData(playerID, true, requestFromBackend);
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
	protected void OnPlayerAuditSuccess(int playerId)
	{
		//We create the player's PlayerData here
		GetPlayerData(playerId);
		
		//Add listener to OnEntityChanged
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (playerController)
			playerController.m_OnControlledEntityChanged.Insert(OnPlayerEntityChanged);
		
		//And then let the modules handle the newly connected player if they need to (they don't atm)
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerAuditSuccess(playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerEntityChanged(IEntity from, IEntity to)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnControlledEntityChanged(from, to);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDisconnected(int playerId)
	{
		SCR_PlayerData playerDisconnectedData = GetPlayerData(playerId);
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerDisconnected(playerId);
		}
		
		playerDisconnectedData.StoreProfile();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerSpawned(playerId, controlledEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerKilled(playerId, player, killer);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].Update(owner, timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! We call SessionIsReady when the backend session is ready
	//! because that's the time when we can check whether the server has writing privileges
	//! If the server has no writing privileges, we don't bother tracking their performance
	void SessionIsReady()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		BackendApi ba = GetGame().GetBackendApi();
		
		#ifdef DEBUG_CAREER		
			//This is a debugging visual display. Only available on clients
			CreateStatVisualizations();
		#else
			//These are all the comprobations to make sure that this server has writing rights. If not, there's no need for a data collector
			if (!gameMode || !ba || !IsMaster())
				return;
			
			SessionStorage baStorage = ba.GetStorage();
			
			if (!baStorage)
				return;
			
			if (!baStorage.GetOnlineWritePrivilege())
			{
				Print("DataCollectorComponent: SessionIsReady: This server has no writing privileges.", LogLevel.DEBUG);
				return;
			}
		#endif
		
		//Invokers that do not belong to the entity are handled here
		gameMode.GetOnPlayerAuditSuccess().Insert(OnPlayerAuditSuccess);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		
		//RPL invoker to show the player's performance in the game mode end screen
		gameMode.GetOnGameModeEnd().Insert(ReplicatePlayerData);
		
		if (!m_Owner)
		{
			Print("DataCollectorComponent: SessionIsReady: m_Owner is null. Can't add the EntityEvent.FRAME flag thus data collector will not work properly.", LogLevel.ERROR);
		}
		else
		{
			SetEventMask(m_Owner, EntityEvent.FRAME);
			m_Owner.SetFlags(EntityFlags.ACTIVE, true);
		}
		
		#ifdef DEBUG_CAREER
			return;
		#endif
		
		//Now we check if there is any player already to create playerData manually
		if (GetGame().GetPlayerManager().GetPlayerCount() <= 0)
			return;
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			m_mPlayerData.Insert(playerId, new SCR_PlayerData(playerId, true));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode)
			return;
		
		//If there is a data collector instance already, return
		if (GetGame().GetDataCollector())
			return;
		
		//If there's no previous data collector instance registered: Register this one
		GetGame().RegisterDataCollector(this);
		
		m_Owner = owner;
		
		BackendApi ba = GetGame().GetBackendApi();
		
		if(!ba)
			return;
		
		if (!ba.IsActive())
		{
			gameMode.GetOnGameStart().Insert(SessionIsReady);
			return;
		}
		
		SessionIsReady();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DataCollectorUI GetUIComponent()
	{
		return m_UiComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//Prototyping method. Modules look for a DEBUG_CAREER boolean defined at DataCollectorComponent or through CLI
	protected void CreateStatVisualizations()
	{	
		m_UiComponent = SCR_DataCollectorUI.Cast(GetOwner().FindComponent(SCR_DataCollectorUI));
		
		if (!m_UiComponent)
			return;
		
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].CreateVisualization();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_DataCollectorComponent()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerAuditSuccess().Remove(OnPlayerAuditSuccess);
		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}
};