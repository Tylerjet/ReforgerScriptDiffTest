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
	/*!
	Check if the system is enabled.
	\return True if the system is enabled
	*/
	bool PlayerSpawnPointsEnabled()
	{
		return m_bEnablePlayerSpawnPoints;
	}
	
	protected void AddSpawnPoint(int playerID)
	{
		if (!m_bEnablePlayerSpawnPoints || !m_SpawnPointPrefab)
			return;
		
		SCR_PlayerSpawnPoint spawnPoint = SCR_PlayerSpawnPoint.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_SpawnPointPrefab), GetOwner().GetWorld()));
		spawnPoint.SetPlayerID(playerID);
		
		m_SpawnPoints.Insert(playerID, spawnPoint);
	}
	protected void RemoveSpawnPoint(int playerID)
	{
		SCR_PlayerSpawnPoint spawnPoint;
		if (m_SpawnPoints.Find(playerID, spawnPoint))
			RplComponent.DeleteRplEntity(spawnPoint, false);
	}
	
	override void OnPlayerConnected(int playerId)
	{
		AddSpawnPoint(playerId);
	}
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		RemoveSpawnPoint(playerId);
	}

	void ~SCR_PlayerSpawnPointManagerComponent()
	{
		foreach (int id, SCR_PlayerSpawnPoint sp : m_SpawnPoints)
		{
			delete sp;
		}
	}
};
