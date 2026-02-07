[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
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
	SCR_PlayerData GetPlayerData(int playerID)
	{
		SCR_PlayerData playerData = m_mPlayerData.Get(playerID);
		if (!playerData)
		{
			playerData = new SCR_PlayerData(playerID);
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
	protected void OnPlayerConnected(int playerId)
	{
		GetPlayerData(playerId);
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].OnPlayerConnected(playerId);
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
		if (!IsMaster())
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}
		
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].Execute(owner, timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{	
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		//No gameMode means the gameMode is not officialy approved and thus it's unsupported.
		if (!gameMode)
			return;
		
		//Is this a Server? If not, return
		if(!IsMaster())
			return;
		
		//If there is a data collector instance already, return
		if (GetGame().GetDataCollector())
			return;
		
		GetGame().RegisterDataCollector(this);
		
		CreateStatVisualizations(); //Prototyping: Text layouts on screen with the different statistics
		
		SetEventMask(owner, EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		
		//Invokers that do not belong to the entity are handled here
		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnected);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		
		//Now we check if there is any player already to create playerData manually
		if (GetGame().GetPlayerManager().GetPlayerCount()<=0)
			return;
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			m_mPlayerData.Insert(playerId, new SCR_PlayerData(playerId));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DataCollectorUI GetUIComponent()
	{
		return m_UiComponent;
	}
	
	//Prototyping method. Checks for a #define flag on DataCollectorModule
	//------------------------------------------------------------------------------------------------
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
		
		gameMode.GetOnPlayerConnected().Remove(OnPlayerConnected);
		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}

}
