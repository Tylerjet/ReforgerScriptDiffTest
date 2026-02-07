[ComponentEditorProps(category: "GameScripted/Network", description: "")]
class SCR_NotificationsComponentClass: ScriptComponentClass
{
};

/*!
Framework for sending notifications to players.
*/
class SCR_NotificationsComponent : ScriptComponent
{
	//Vizualization
	[Attribute(desc: "Any notifications that only need basic text no references")]
	protected ref array<ref SCR_NotificationDisplayData> m_aNotificationDisplayData;
	
	protected ref array<ref SCR_NotificationData> m_aHistory = new array<ref SCR_NotificationData>;
	protected ref ScriptInvoker Event_OnNotification = new ScriptInvoker;
	
	protected SCR_NotificationData m_LastNotificationWithLocation;
	
	//Notification Display Data Map
	ref map<ENotification, SCR_NotificationDisplayData> m_NotificationDisplayDataMap = new map<ENotification, SCR_NotificationDisplayData>;
	
	protected bool m_bIsUpdatingNotificationData;
	
	//How long each notification exists before it gets deleted, Time in seconds
	static const int NOTIFICATION_DELETE_TIME = 30;
	
	//How many notifications are remembered in history.
	static const int NOTIFICATION_HISTORY_LENGHT = 10;
	
	//Hot fix for when player names cannot be found
	protected ref map <int, string> m_mPlayerNameHistory = new ref map <int, string>;

	/*!
	Get invoker called on player's machine when a notification is received
	\return Script invoker
	*/
	ScriptInvoker GetOnNotification()
	{
		return Event_OnNotification;
	}
	
	/*!
	Get the list of previous notifications. (new to old)
	\param[out] outHistory Array to be filled with notifications, newest first
	\return Number of notifications
	*/
	int GetHistoryNewToOld(out notnull array<SCR_NotificationData> outHistory)
	{
		int count = m_aHistory.Count();
		for (int i = 0; i < count; i++)
		{
			outHistory.Insert(m_aHistory[i]);
		}
		return count;
	}
	
	/*!
	Get the list of previous notifications. (old to new)
	\param[out] outHistory Array to be filled with notifications, oldest first
	\param how far back the history needs to be taken from. -1 means this is ignored
	\return Number of notifications
	*/
	int GetHistoryOldToNew(out notnull array<SCR_NotificationData> outHistory, int maxHistoryIndex = -1)
	{
		int count = m_aHistory.Count();
		if (maxHistoryIndex > 0 && count > maxHistoryIndex)
			count = maxHistoryIndex;
		
		for (int i = count -1; i >= 0; i--)  
		{  
		   outHistory.Insert(m_aHistory[i]);
		}    
		return count;
	}
	
	/*!
	Get the the specific history entry
	\param index int index of history entry
	\param[out] data SCR_NotificationData of the history entry with given index
	\return false if index was not found
	*/
	bool GetHistoryEntry(int index, out SCR_NotificationData data = null)
	{		
		if (index < m_aHistory.Count())
		{
			data = m_aHistory[index];
			return true;
		}
		else 
		{
			return false;
		}
	}
	
	/*!
	Get last ping location
	\param[out] lastPingLocation last ping location vector
	\return false if no last location
	*/
	bool GetLastNotificationLocation(out vector lastLocation)
	{
		if (!m_LastNotificationWithLocation || !m_LastNotificationWithLocation.GetDisplayData())
			return false;
		
		return m_LastNotificationWithLocation.GetDisplayData().GetPosition(m_LastNotificationWithLocation, lastLocation);
	}
	
	/*!
	Get local instance of the notifications system.
	\return Notifications component
	*/
	static SCR_NotificationsComponent GetInstance()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
			return SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
		else
			return null;
	}
	
	
	/*!
	Send notification to all players
	\param notificationID ID of the notification MessageBox
  	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToEveryone(ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendToEveryone(notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to all players
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToEveryone(ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.EVERYONE, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position);
		return SendToEveryoneData(notificationID, newNotificationData);
	}
	
	//Set the actual data to players
	protected static bool SendToEveryoneData(ENotification notificationID, SCR_NotificationData data)
	{
		//--- Only server can broadcast messages
		if (!Replication.IsServer()) return false;
		
		array<int> players = new array<int>;
		for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
		{
			SendToPlayerData(players[i], notificationID, data);
		}
		return true;
	}
	
	/*!
	Send notification to player with given ID
	\param playerID Id of the recipient
	\param notificationID ID of the notification message
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToPlayer(int playerID, ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendToPlayer(playerID, notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to player with given ID
	\param playerID Id of the recipient
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToPlayer(int playerID, ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.SPECIFIC_PLAYER, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position); 
		return SendToPlayerData(playerID, notificationID, newNotificationData);
	}
	
	//Set the actual data to player
	protected static bool SendToPlayerData(int playerID, ENotification notificationID, SCR_NotificationData data)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return false;
		
		SCR_NotificationsComponent notificationsComponent = SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
		if (!notificationsComponent)
			return false;
		
		notificationsComponent.SendToOwner(notificationID, data);
		return true;
	}
	
	/*!
	Send notification to all players that have GM rights
	\param notificationID ID of the notification MessageBox
  	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGameMasters(ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendToGameMasters(notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to all players that have GM rights
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGameMasters(ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.GM_ONLY, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position);
		return SendToGameMastersData(notificationID, newNotificationData);
	}
	
	//Set the actual data to players that are GM
	protected static bool SendToGameMastersData(ENotification notificationID, SCR_NotificationData data, int playerID = -1)
	{
		//--- Only server can broadcast messages
		if (!Replication.IsServer()) return false;
		
		array<int> players = new array<int>;
		for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
		{		
			if (playerID == players[i])
			{
				SendToPlayerData(players[i], notificationID, data);
				continue;
			}
				
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (!core) continue;
		
			SCR_EditorManagerEntity editorManager = core.GetEditorManager(players[i]);
			if (!editorManager) continue;
			
			if (!editorManager.IsLimited())
			{
				SendToPlayerData(players[i], notificationID, data);
			}	
		}
		return true;
	}
	
	/*!
	Send notification to all players that have GM rights
	\param notificationID ID of the notification MessageBox
  	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGameMastersAndPlayer(int playerID, ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendToGameMastersAndPlayer(playerID, notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to all players that have GM rights
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGameMastersAndPlayer(int playerID, ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.GM_OR_AFFECTED_PLAYER_ONLY, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position);
		return SendToGameMastersData(notificationID, newNotificationData, playerID);
	}
	
	/*!
	Send notification to all players in a group
	\param groupID ID of group which players need to be a part of to recieve the notification
	\param notificationID ID of the notification MessageBox
  	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGroup(int groupID, ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendToGroup(groupID, notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to all players in a group
	\param groupID ID of group which players need to be a part of to recieve the notification
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendToGroup(int groupID, ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.PLAYER_GROUP, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position);
		return SendToGroupData(notificationID, newNotificationData, groupID);
	}
	
	//Set the actual data to players in the given group
	protected static bool SendToGroupData(ENotification notificationID, SCR_NotificationData data, int groupID)
	{
		//--- Only server can broadcast messages
		if (!Replication.IsServer()) return false;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;
		
		SCR_AIGroup group = groupManager.FindGroup(groupID);
		if (!group)
			return false;
		
		array<int> players = group.GetPlayerIDs();
		
		if (players.IsEmpty())
			return false;
		
		foreach (int playerID: players)
		{
			SendToPlayerData(playerID, notificationID, data);
		}
		
		return true;
	}

	/*!
	Send notification to the player on this machine.
	\param notificationID ID of the notification message
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocal(ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendLocal(notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to the player on this machine.
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocal(ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.LOCAL_ONLY, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position); 
		return SendLocalData(notificationID, newNotificationData);
	}
	
	/*!
	Send notification to the player on this machine (if has Game master rights).
	\param notificationID ID of the notification message
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocalGameMaster(ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendLocalGameMaster(notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to the player on this machine (if has Game master rights).
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocalGameMaster(ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || editorManager.IsLimited())
			return false;
		
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.LOCAL_GM_ONLY, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position); 
		return SendLocalData(notificationID, newNotificationData);
	}
	
		/*!
	Send notification to the player on this machine (if has Game master rights).
	\param notificationID ID of the notification message
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocalNonGameMaster(ENotification notificationID, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		return SendLocalNonGameMaster(notificationID, vector.Zero, param1, param2, param3, param4, param5);
	}
	
	/*!
	Send notification to the player on this machine (if has Game master rights).
	\param notificationID ID of the notification message
	\param position notification position
	\param param1
	\param param2
	\param param3
	\param param4
	\param param5
	\return True if the notification was sent successfully
	*/
	static bool SendLocalNonGameMaster(ENotification notificationID, vector position, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || !editorManager.IsLimited())
			return false;
		
		SCR_NotificationData newNotificationData = SCR_NotificationData();
		newNotificationData.SetParameters(ENotificationReceiver.LOCAL_NON_GM_ONLY, param1, param2, param3, param4, param5);
		newNotificationData.SetPosition(position); 
		return SendLocalData(notificationID, newNotificationData);
	}
	
	//Send actual notification data localy
	protected static bool SendLocalData(ENotification notificationID, SCR_NotificationData data)
	{
		SCR_NotificationsComponent notificationsComponent = GetInstance();
		if (!notificationsComponent)
			return false;
		
		notificationsComponent.ReceiveSCR_NotificationData(notificationID, data);//position, target);
		return true;
	}
	
	
	/*!
	Send notification to player who owns this player controller.
	\param id ID of the notification message
	\param data Notification data
	\return True if the notification was sent successfully
	*/
	bool SendToOwner(ENotification id, SCR_NotificationData data = null)
	{
		if (data)
			Rpc(ReceiveSCR_NotificationData, id, data);
		else
			Rpc(ReceiveNotification, id); //--- Don't send data when it's not needed (replication complains when null is received)
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ReceiveNotification(ENotification id)
	{
		ReceiveSCR_NotificationData(id, null);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ReceiveSCR_NotificationData(ENotification id, SCR_NotificationData data)
	{
		SCR_NotificationData entry = data;
		if (!entry) entry = new SCR_NotificationData;
		
		//--- Set Meta data
		entry.SetMeta(id, GetNotificationDisplayData(id));
		
		//--- Save to history
		m_aHistory.InsertAt(entry, 0);
		m_aHistory.Resize(Math.Min(m_aHistory.Count(), NOTIFICATION_HISTORY_LENGHT));
		
		//--- Trigger event to be captured by other systems
		Event_OnNotification.Invoke(entry);
		//entry.Log();
		
		vector position;
		data.GetPosition(position);
		
		if (position != vector.Zero)
			m_LastNotificationWithLocation = data;
		
		//Start updating notification times if not yet done
		if (!m_bIsUpdatingNotificationData)
			UpdateNotificationData(true, GetOwner());
	}
	
	//======================== GET NOTIFICATION DISPLAY DATA ========================\\
	protected SCR_NotificationDisplayData GetNotificationDisplayData(ENotification notificationID)
	{		
		if (m_NotificationDisplayDataMap.Contains(notificationID))
		{
			return m_NotificationDisplayDataMap.Get(notificationID);
		}
		else 
		{
			Print("Notification data not found  in 'SCR_NotificationsComponent' for key: '" + typename.EnumToString(ENotification, notificationID) + "'.", LogLevel.WARNING);
			return m_NotificationDisplayDataMap.Get(ENotification.UNKNOWN);
		}
	}
	
	//======================== BUILD NOTIFICATION INFO MAP ========================\\
	protected void GenerateNotificationDisplayDataMap()
	{
		for(int i = 0; i < m_aNotificationDisplayData.Count(); i++)
        {
			if (!m_NotificationDisplayDataMap.Contains(m_aNotificationDisplayData[i].m_NotificationKey))
            	m_NotificationDisplayDataMap.Set(m_aNotificationDisplayData[i].m_NotificationKey, m_aNotificationDisplayData[i]);
			else
				Print("Notification data in 'SCR_NotificationsLogComponent' has duplicate notification info key: '" + typename.EnumToString(ENotification, m_aNotificationDisplayData[i].m_NotificationKey) + "'. There should only be one of each key!", LogLevel.WARNING);
        }
	}
	
	//======================== PLAYER NAME HOTFIX ========================\\
	/*!
	Hot fix to Get player names when the name cannot be found anymore in the playerManager
	\param playerID id of player to get name from
	\return saved player name
	*/
	string GetPlayerNameFromHistory(int playerID)
	{
		string playerName = string.Empty;
		m_mPlayerNameHistory.Find(playerID, playerName);
			
		return playerName;
	}
	
	//~Hotfix to get player names when the player disconnected
	protected void AddPlayerNameToHistory(int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		m_mPlayerNameHistory.Set(playerID, playerManager.GetPlayerName(playerID));
	}

	//======================== START LISTENING TO UPDATE ========================\\
	protected void UpdateNotificationData(bool updateNotificationData, IEntity owner)
	{
		if (updateNotificationData == m_bIsUpdatingNotificationData)
			return;
		
		m_bIsUpdatingNotificationData = updateNotificationData;
		
		if (m_bIsUpdatingNotificationData)
			SetEventMask(owner, EntityEvent.FRAME);
		else 
			ClearEventMask(owner, EntityEvent.FRAME);
	}
	
	//======================== UPDATE ========================\\
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		int count = m_aHistory.Count();

		for(int i = 0; i < count; i++)
        {
           if (!m_aHistory[i])
				continue;
			
			//Update the time left and check if it needs to be deleted
			if (m_aHistory[i].UpdateNotificationData(timeSlice))
			{
				m_aHistory.RemoveOrdered(i);
				i--;
				count--;
			}
        }
		
		if (m_aHistory.IsEmpty())
			UpdateNotificationData(false, owner);
	}
	
	//======================== ON INIT ========================\\
	override void OnPostInit(IEntity owner)
	{
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController)
		{
			return;
		}
		
		GenerateNotificationDisplayDataMap();
		
		if (!m_aHistory.IsEmpty())
			UpdateNotificationData(true, owner);
		
		//~Hotfix to remember player names even when the player left
		if (Replication.IsClient())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerRegistered().Insert(AddPlayerNameToHistory);
			
			//~ Get a list of connected players as OnPostInit might be called after connected players were registered
			array<int> players = new array<int>;
			
			GetGame().GetPlayerManager().GetPlayers(players);
			
			foreach (int player: players)
				AddPlayerNameToHistory(player);
		}
	}
	
	override void OnDelete(IEntity owner)
	{
		//~Hotfix to remember player names even when the player left
		if (Replication.IsClient())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerRegistered().Remove(AddPlayerNameToHistory);
		}
	}
};


