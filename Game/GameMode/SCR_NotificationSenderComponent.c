[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.", color: "0 0 255 255")]
class SCR_NotificationSenderComponentClass: SCR_BaseGameModeComponentClass
{
};


class SCR_NotificationSenderComponent : SCR_BaseGameModeComponent
{		
	[Attribute("1", desc: "Show player left notification if player left. Hotfix for editor as this system does not know if player had editor rights or not!")]
	protected bool m_bShowPlayerLeftNotification;
	
	[Attribute("10", desc: "Is killfeed enabled and what info will be displayed (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedType))]
	protected EKillFeedType m_iKillFeedType;
	
	[Attribute("40", desc: "If killfeed is enabled, what is the relationship between the player that died and the local player. (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedReceiveType))]
	protected EKillFeedReceiveType m_iReceiveKillFeedType;
	
	protected Faction m_FactionOnSpawn;
	
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	
	//States
	protected bool m_bListeningToWeatherChanged;
	
	override void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
		//~ hot fix for On Controllable Destroyed issues \/
		if (Replication.IsClient())
			return;
		
		if (!entity)
			return;
		
		RplComponent entityRpl = RplComponent.Cast(entity.FindComponent(RplComponent));
		RplComponent instigatorRpl;
		
		if (instigator)
			instigatorRpl = RplComponent.Cast(instigator.FindComponent(RplComponent));
		
		RplId entityId = RplId.Invalid();
		RplId instigatorId = RplId.Invalid();
		
		if (entityRpl)
			entityId = entityRpl.Id();
		
		if (instigatorRpl)
			instigatorId = instigatorRpl.Id();
		
		OnControllableDestroyedBroadCast(entityId, instigatorId);
		Rpc(OnControllableDestroyedBroadCast, entityId,instigatorId);
		
		//~ hot fix for On Controllable Destroyed issues /\
	}
	
	
	//~ Todo: This is a hot a hotfix for On Controllable Destroyed issues \/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnControllableDestroyedBroadCast(RplId entityId, RplId instigatorId)
	{
		IEntity entity;
		IEntity instigator;
		
		RplComponent entityRpl = RplComponent.Cast(Replication.FindItem(entityId));
		RplComponent instigatorRpl = RplComponent.Cast(Replication.FindItem(instigatorId));
		
		if (entityRpl)
			entity = entityRpl.GetEntity();
		
		if (instigatorRpl)
			instigator = instigatorRpl.GetEntity();
		
		OnControllableDestroyedHotfix(entity, instigator);
	}
	//~ hot fix for On Controllable Destroyed  issues /\
	
	//~ Todo: hot fix for On Controllable Destroyed  issues - Move logic to OnControllableDestroyed if fixed
	protected void OnControllableDestroyedHotfix(IEntity entity, IEntity instigator)
	{		
		//~	No entity destroyed
		if (!entity)
			return;
		
		bool isUnlimitedEditorOpened = false;
		
		//~ Check if player has unlimited editor and if the editor is open
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			isUnlimitedEditorOpened = !editorManager.IsLimited() && editorManager.IsOpened();
		
		//~ Killfeed is disabled and unlimited editor is not open
		if (!isUnlimitedEditorOpened && m_iKillFeedType == EKillFeedType.DISABLED)
			return;
		
		int playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity);
		if (playerId <= 0)
			return;
		
		//~ Check if killed player message can be seen if Limited editor
		if (!isUnlimitedEditorOpened && m_iReceiveKillFeedType != EKillFeedReceiveType.ALL)
		{
			Faction localPlayerFaction = m_RespawnSystemComponent.GetLocalPlayerFaction();

			//~ No local faction so don't show killfeed
			if (!localPlayerFaction)
			{
				return;
			}
			
			int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
			Faction killedPlayerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);
			
			switch (m_iReceiveKillFeedType)
			{
				//~ check if in group
				case EKillFeedReceiveType.GROUP_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always in the same group
					if (localPlayerID != playerId)
					{
						//~ Factions not friendly so don't show killfeed
						if (!localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
							return;
						
						//~ No group manager so don't send
						SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
						if (!groupManager)
							return;
						
						SCR_AIGroup localPlayerGroup = groupManager.GetPlayerGroup(localPlayerID);
						
						//~ If not in group or not in the same group do not send
						if (!localPlayerGroup || !localPlayerGroup.IsPlayerInGroup(playerId))
							return;
					}
					
					break;
				}
				//~ Check if the same faction
				case EKillFeedReceiveType.SAME_FACTION_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always the same faction
					if (localPlayerID != playerId)
					{
						//~ If no local faction or if not the same faction do not show killfeed
						if (!localPlayerFaction || localPlayerFaction != killedPlayerFaction)
							return;
					}
					
					break;
				}
				//~ Check if allies
				case EKillFeedReceiveType.ALLIES_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always allied
					if (localPlayerID != playerId)
					{
						//~ Factions not friendly so don't show killfeed
						if (!localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
							return;
					}
					
					break;
				}
				//~ Check if enemies
				case EKillFeedReceiveType.ENEMIES_ONLY :
				{
					//~ If local player killed it is never an enemy
					if (localPlayerID == playerId)
						return;
					
					//~ Factions friendly so don't show killfeed
					if (localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
						return;
					
					break;
				}
			}
		}
		
		//~ Never show killer so simply show player died if limited editor
		if (!isUnlimitedEditorOpened && m_iKillFeedType == EKillFeedType.UNKNOWN_KILLER)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			return;
		}
		
		int killerId;
		if (entity == instigator)
			killerId = playerId;
		else 
			killerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(instigator);

		SCR_EditableCharacterComponent killerEditableCharacterComponent;
		//~ If there is a killer and the killer is not the player itself
		if (instigator && killerId <= 0)
		{
			killerEditableCharacterComponent = SCR_EditableCharacterComponent.Cast(instigator.FindComponent(SCR_EditableCharacterComponent));
		
			//~ Killer was not character so get killer in vehicle
			if (!killerEditableCharacterComponent)
				killerEditableCharacterComponent = GetKillerFromVehicle(instigator, instigator.IsInherited(Vehicle));
			
			if (killerEditableCharacterComponent)
				killerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(killerEditableCharacterComponent.GetOwner());
		}

		bool playerIsPossessed = false; 
		bool killerIsPossessed = false; 
		
		//~ Check if player or killer where possessed
		SCR_PossessingManagerComponent possesionManager = SCR_PossessingManagerComponent.GetInstance();
		if (possesionManager)
		{
			playerIsPossessed = possesionManager.IsPossessing(playerId);
			
			if (killerId > 0)
				killerIsPossessed = possesionManager.IsPossessing(killerId);
		}
			
		//Death notification	
		//Suicide	
		if (playerId == killerId || !instigator)
		{			
			//Player Suicide
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			//Possessed Suicide
			else 
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.POSSESSED_AI_DIED, playerId);
		}
		//If killed by other player
		else if (killerId > 0)
		{
			//Player killed player
			if (!playerIsPossessed && !killerIsPossessed)
			{
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_KILLED_PLAYER, killerId, playerId);
			}
			//Possesed player killed by other player (Show player killed by NPC and for GM: possesed GM killed player)
			else if (!playerIsPossessed && killerIsPossessed)
			{		
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.POSSESSED_AI_KILLED_PLAYER, killerId, playerId);
				SCR_NotificationsComponent.SendLocalNonGameMaster(ENotification.AI_KILLED_PLAYER, killerId, playerId);
			}
			//Player killed possessed player (Show to GM only)
			else if (playerIsPossessed && !killerIsPossessed)
			{
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.PLAYER_KILLED_POSSESSED_AI, killerId, playerId);
			}
			//Possessed AI Killed Possessed AI (GM Only)
			else
			{
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.POSSESSED_AI_KILLED_POSSESSED_AI, killerId, playerId);
			}
		}
		//Killed by NPC
		else if (killerEditableCharacterComponent)
		{
			int killerRplId = Replication.FindId(killerEditableCharacterComponent);
			
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.AI_KILLED_PLAYER, killerRplId, playerId);
			else 
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.AI_KILLED_POSSESSED_AI, killerRplId, playerId);
		}
		//Unknown killer
		else 
		{
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			else 
				SCR_NotificationsComponent.SendLocalGameMaster(ENotification.POSSESSED_AI_DIED, playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EditableCharacterComponent GetKillerFromVehicle(IEntity veh, bool pilot)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
			
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
		
		for (int i = 0, compart = compartmentManager.GetCompartments(compartments); i < compart; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			
			if (pilot && slot.Type() == PilotCompartmentSlot)
			{
				if (slot.GetOccupant())
					return SCR_EditableCharacterComponent.Cast(slot.GetOccupant().FindComponent(SCR_EditableCharacterComponent));
			}
			else if (!pilot && slot.Type() == TurretCompartmentSlot)
			{
				if (slot.GetOccupant())
					return SCR_EditableCharacterComponent.Cast(slot.GetOccupant().FindComponent(SCR_EditableCharacterComponent));
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChanged(bool isLimited)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) 
			return;
		
		int playerID = editorManager.GetPlayerID();
		
		//Became Player
		if (isLimited)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_NO_LONGER_GM, playerID);
		//Became GM
		else 
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_BECAME_GM, playerID);
	}
	
	override void OnPlayerRegistered(int playerId) 
	{
		//Join notification
		SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_JOINED, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId) 
	{
		//~ Should never be called for clients but just in case
		if (!m_bShowPlayerLeftNotification || !GetGameMode().IsMaster())
			return;
		
		SCR_EditorManagerEntity editorManager;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
		{
			 editorManager = core.GetEditorManager(playerId);
		}

		//Leave Notification GM
		if (editorManager && !editorManager.IsLimited())
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_GM_LEFT, playerId);
		//Leave notification Player
		else
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_LEFT, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		//Check if faction changed and send notification if diffrent
		if (m_RespawnSystemComponent) 
		{
			//Get factions using ID
			Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);
		
			//Faction is diffrent
			if (m_FactionOnSpawn != playerFaction)
			{
				//Send player joined faction notification
				SCR_NotificationsComponent.SendToGameMasters(ENotification.PLAYER_JOINED_FACTION, playerId);
				m_FactionOnSpawn = playerFaction;
			}
		}
	}	
	
	

	
	//======================================== WEATHER SET TO LOOPING ========================================\\
	/*!
	Called when weather is set to looping or when weather itself is changed (which auto sets it on looping)
	This is to make sure the notification, that weather is set, is only called once
	*/
	void OnWeatherChangedNotification(int playerID)
	{
		if (Replication.IsClient())
			return;
		
		if (!m_bListeningToWeatherChanged)
		{
			m_bListeningToWeatherChanged = true;
			GetGame().GetCallqueue().CallLater(OnWeatherChangedNotificationDelay, 0 , false, playerID);
		}
	}
	
	protected void OnWeatherChangedNotificationDelay(int playerID)
	{
		m_bListeningToWeatherChanged = false;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		WeatherState currentState = new WeatherState();
		weatherManager.GetCurrentWeatherState(currentState);
		
		if (!currentState)
			return;
		
		SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_WEATHER_CHANGED, playerID, currentState.GetStateID());
	}
	
	
	//======================================== SET SHOW KILLFEED ========================================\\	
	/*!
	Get the current killfeed type
	\return the type of killfeed that is displayed
	*/
	EKillFeedType GetKillFeedType()
	{
		return m_iKillFeedType;
	}
	
	/*!
	Server set killfeed type
	\param killFeedType new killfeed type to set
	\param playerNotificationId add player ID of player that changed the type to display a notification
	*/
	void SetKillFeedType(EKillFeedType killFeedType, int playerNotificationId = -1)
	{
		if (!GetGameMode().IsMaster() || killFeedType == m_iKillFeedType)
			return;
		
		SetKillFeedTypeBroadcast(killFeedType);
		Rpc(SetKillFeedTypeBroadcast, killFeedType);
		
		/*if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_TYPE, playerNotificationId, killFeedType);*/
	}
	
	/*!
	Get the current killfeed receive type
	\return the type from whom the player receives killfeed
	*/
	EKillFeedReceiveType GetReceiveKillFeedType()
	{
		return m_iReceiveKillFeedType;
	}
	
	/*!
	Server set killfeed receive type
	\param receiveKillFeedType new killfeed reveive type to set
	\param playerNotificationId add player ID of player that changed the type to display a notification
	*/
	void SetReceiveKillFeedType(EKillFeedReceiveType receiveKillFeedType, int playerNotificationId = -1)
	{
		if (!GetGameMode().IsMaster() || receiveKillFeedType == m_iReceiveKillFeedType)
			return;
		
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		Rpc(SetReceiveKillFeedTypeBroadcast, receiveKillFeedType);
		
		/*if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_RECEIVE_TYPE, playerNotificationId, receiveKillFeedType);*/
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetKillFeedTypeBroadcast(EKillFeedType killFeedType)
	{
		m_iKillFeedType = killFeedType;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetReceiveKillFeedTypeBroadcast(EKillFeedReceiveType receiveKillFeedType)
	{
		m_iReceiveKillFeedType = receiveKillFeedType;
	}
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
       	writer.WriteInt(m_iKillFeedType); 
       	writer.WriteInt(m_iReceiveKillFeedType); 
		
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		EKillFeedType killFeedType;
		EKillFeedReceiveType receiveKillFeedType;
		
        reader.ReadInt(killFeedType);
        reader.ReadInt(receiveKillFeedType);

		SetKillFeedTypeBroadcast(killFeedType);
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		
        return true;
    }
	
	//======================================== INIT ========================================\\
	override void EOnInit(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
		
		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		
		super.EOnInit(owner);
	}
	
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
		
		super.OnPostInit(owner);
	}
	
	
	//======================================== DELETE ========================================\\
	protected override void OnDelete(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Remove(OnEditorLimitedChanged);
		
		super.OnDelete(owner);
	}
};

/*!
What kind of killfeed will be displayed in the gamemode. 
Noted that players with unlimited Editor will always see full killfeed if Editor is open
*/
enum EKillFeedType
{
	DISABLED = 0, ///< Killfeed is disabled
	UNKNOWN_KILLER = 10, ///< Will see killfeed messages but will never know who the killer is
	FULL = 20, ///< Will see every player (except possessed pawns) killed messages. Including killer
};


/*!
From who will players receive kill notifications
Noted that players with unlimited Editor will always see full killfeed if Editor is open
*/
enum EKillFeedReceiveType
{
	ALL = 0, ///< Will see killfeed from allies and enemies alike
	ENEMIES_ONLY = 10, ///< Will see killfeed from enemies only
	ALLIES_ONLY = 20, ///< Will see killfeed from allies only
	SAME_FACTION_ONLY = 30, ///< Will see killfeed from same faction only
	GROUP_ONLY = 40, ///< Will see killfeed from group members only
};


