[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerSpawnPointManagerComponentClass: SCR_BaseGameModeComponentClass
{
};
class SCR_PlayerSpawnPointManagerComponent: SCR_BaseGameModeComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_SpawnPointPrefab;
	
	[Attribute("1")]
	protected bool m_bEnablePlayerSpawnPoints;
	
	protected ref map<int, SCR_PlayerSpawnPoint> m_SpawnPoints = new map<int, SCR_PlayerSpawnPoint>();
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set status of automatic player spawn points.
	When enabled, a spawn point will be created for every connected player, even those who connect later.
	When disabled, all existing player spawn points will be deleted.
	\param enable True to enable the system
	\param notifcationPlayerID if not -1 then a notification will be shown to all players that spawning on players is enabled/disabled
	*/
	void EnablePlayerSpawnPoints(bool enable, int notifcationPlayerID = -1)
	{
		if (enable == m_bEnablePlayerSpawnPoints)
			return;
		
		m_bEnablePlayerSpawnPoints = enable;
		
		if (m_bEnablePlayerSpawnPoints)
		{
			//--- Create spawn points for all connected players
			array<int> playerIDs = {};
			int playerIDCount = GetGame().GetPlayerManager().GetPlayers(playerIDs);
			for (int i; i < playerIDCount; i++)
			{
				AddSpawnPoint(playerIDs[i]);
			}
		}
		else
		{
			//--- Delete all existing spawn points
			for (int i, count = m_SpawnPoints.Count(); i < count; i++)
			{
				delete m_SpawnPoints.Get(i);
			}
			m_SpawnPoints.Clear();
		}
		
		if (notifcationPlayerID > 0)
		{
			if (enable)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_PLAYER, notifcationPlayerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_PLAYER, notifcationPlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if the system is enabled.
	\return True if the system is enabled
	*/
	bool IsPlayerSpawnPointsEnabled()
	{
		return m_bEnablePlayerSpawnPoints;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddSpawnPoint(int playerID)
	{
		if (!m_bEnablePlayerSpawnPoints || !m_SpawnPointPrefab)
			return;
		
		Resource resource = Resource.Load(m_SpawnPointPrefab);
		if (!resource || !resource.IsValid())
		{
			Debug.Error(string.Format("Invalid spawn point resource: %1 in %2::AddSpawnPoint", 
						m_SpawnPointPrefab, Type().ToString()));
			
			return;
		}
		
		SCR_PlayerSpawnPoint spawnPoint = SCR_PlayerSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetOwner().GetWorld()));
		spawnPoint.SetPlayerID(playerID);
		
		if (!spawnPoint) // Don't register empty entries
		{
			Debug.Error(string.Format("PlayerSpawnPoint for player: %1 could not be created!", playerID));
			return;
		}
		
		m_SpawnPoints.Insert(playerID, spawnPoint);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveSpawnPoint(int playerID)
	{
		SCR_PlayerSpawnPoint spawnPoint;
		if (m_SpawnPoints.Find(playerID, spawnPoint))
		{
			RplComponent.DeleteRplEntity(spawnPoint, false);
			m_SpawnPoints.Remove(playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		AddSpawnPoint(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		RemoveSpawnPoint(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		int playerId = requestComponent.GetPlayerId();
		SCR_PlayerSpawnPoint playerPoint;
		if (m_SpawnPoints.Find(playerId, playerPoint))
			playerPoint.EnablePoint(playerId, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		if (!m_pGameMode.IsMaster())
			return;
		
		SCR_PlayerSpawnPoint playerPoint;
		if (m_SpawnPoints.Find(playerId, playerPoint))
			playerPoint.DisablePoint(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		foreach (int pointId, SCR_PlayerSpawnPoint spawnPoint : m_SpawnPoints)
			RplComponent.DeleteRplEntity(spawnPoint, false);
			
	}
};
