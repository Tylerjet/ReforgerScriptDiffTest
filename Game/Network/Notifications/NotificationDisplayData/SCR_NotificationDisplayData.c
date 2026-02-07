/*!
Used to get the display information for each notification.
This is the most basic of the SCR_NotificationDisplayData and can only display a simple string without references.
SCR_NotificationData should not have any parameters set.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationDisplayData
{
	[Attribute("0", UIWidgets.SearchComboBox, "Notification", "", ParamEnumArray.FromEnum(ENotification) )]
	ENotification m_NotificationKey;
	
	[Attribute(desc: "Holds the display information of the notification, Fill in Name and Color. Optional: Icon")]
	ref SCR_UINotificationInfo m_info;	
	
	/*!
	Sets the initial display data such as initial notification position and faction related color of notification
	\param data SCR_NotificationData to get the parameters
	*/
	void SetInitialDisplayData(SCR_NotificationData data)
	{
		//Save Initial positions
		if (m_info.GetEditorSetPositionData() != ENotificationSetPositionData.NEVER_AUTO_SET_POSITION)
			SetPosition(data);
			
		//Save faction related color. Is ignored if no faction related color type is assigned
		SetFactionRelatedColor(data);
	}
	
	/*!
	Returns the display data
	\param data SCR_NotificationData to get the parameters
	\param[out] icon as ResourceName
	\param[out] colorEnum as ENotificationColor to be used by the SCR_NotificationMessageUIComponent to get the color from SCR_NotificationsLogComponent
	*/
	void GetDisplayVisualizationData(SCR_NotificationData data, out SCR_UINotificationInfo info = null, out ENotificationColor colorEnum = ENotificationColor.NEUTRAL)
	{
		info = m_info;
		
		colorEnum = m_info.GetNotificationColor();

		if (colorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			colorEnum = data.GetFactionRelatedColor();
	}
	
	/*!
	Returns the display data. Only valid if UI info is SCR_ColoredTextNotificationUIInfo
	\param data SCR_NotificationData to get the parameters
	\return colorEnum as ENotificationColor to be used by the SCR_NotificationMessageUIComponent to get the color from SCR_NotificationsLogComponent
	*/
	ENotificationColor GetTextColor(SCR_NotificationData data)
	{
		SCR_ColoredTextNotificationUIInfo coloredTextInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		if (!coloredTextInfo)
			return ENotificationColor.NEUTRAL;
		
		ENotificationColor colorEnum = coloredTextInfo.GetNotificationTextColor();

		if (colorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			colorEnum = data.GetFactionRelatedTextColor();
		
		return colorEnum;
	}
	
	/*!
	Returns the display data
	\param data SCR_NotificationData to get the parameters
	\param[out] leftTextColorEnum as ENotificationColor to be used by the SCR_SplitNotificationMessageUIComponent to get the color from SCR_NotificationsLogComponent
	\param[out] rightTextColorEnum as ENotificationColor to be used by the SCR_SplitNotificationMessageUIComponent to get the color from SCR_NotificationsLogComponent
	*/
	void GetSplitNotificationTextColors(SCR_NotificationData data, out ENotificationColor leftTextColorEnum = ENotificationColor.NEUTRAL, out ENotificationColor rightTextColorEnum = ENotificationColor.NEUTRAL)
	{
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		
		if (!splitNotificationUIInfo)
			return;
		
		ENotificationColor leftFactionColor, rightFactionColor;
		data.GetSplitFactionRelatedColor(leftFactionColor, rightFactionColor);
		
		leftTextColorEnum = splitNotificationUIInfo.GetLeftTextColor();
		
		if (leftTextColorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE || splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly())
			leftTextColorEnum = leftFactionColor;
		
		rightTextColorEnum = splitNotificationUIInfo.GetRightTextColor();
		
		if (rightTextColorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			rightTextColorEnum = rightFactionColor;
	}
	
	
	
	/*!
	Returns the display text using the parameters set in SCR_NotificationData
	\param data SCR_NotificationData to get the parameters
	\return string display text as shown in the notification
	*/
	string GetText(SCR_NotificationData data)
	{
		if (!m_info)
		{
			Print("(" + typename.EnumToString(ENotification, data.GetID()) + ") SCR_NotificationDisplayData has no UIInfo assigned!", LogLevel.WARNING); 
			return string.Empty;
		} 
		
		return m_info.GetName();
	}
		
	/*!
	If true it will merge param1 with param2 using param2 as format (eg: Translate(param2, param1)) before constructing the full notification formatted string. Param2 will need %1 to in the localization. 
	Mainly used by TaskStateChanged as the objective name (param2) includes the location name (param1) which need to be formatted before formatting the entire string.
	\return bool True if must merge
	*/
	bool MergeParam1With2()
	{
		return false;
	}
	
	/*!
	Get UI info
	\return string Ui info holding various of display information for the notification
	*/
	SCR_UINotificationInfo GetNotificationUIInfo()
	{
		return m_info;
	}
	
	protected string GetPlayerName(int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return string.Empty;
		
		string playerName = playerManager.GetPlayerName(playerID);
		
		//Player name not found
		if (playerName.IsEmpty())
		{
			SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
			if (notificationsManager)
				playerName = notificationsManager.GetPlayerNameFromHistory(playerID);
		}
			
		return playerName;
	}
	
	protected string GetEditableEntityName(int enditableEntityID)
	{
		//Get target Entity
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(enditableEntityID));
		
		if (entity)
    		return entity.GetDisplayName();
		else
   			return string.Empty;
	}
	
	protected bool GetCharacterName(int enditableEntityID, out string format, out string firstname, out string alias, out string surname)
	{
		//Get target Entity
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(enditableEntityID));
		if (!entity)
    		return false;

		SCR_CharacterIdentityComponent charIdentity = SCR_CharacterIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_CharacterIdentityComponent));
		if (!charIdentity)
    		return false;
		
		charIdentity.GetFormattedFullName(format, firstname, alias, surname);
		return true;
	}
	
	/*!
	Gives a position vector that is linked to the notification. 
	This is for editor functionality so the GM can teleport to the notification positions
	Will return false if specific SCR_NotificationDisplayData does not have a linked position
	\param data SCR_NotificationData to get the parameters
	\param[out] position vector the position linked to the notification
	\return bool returns false if has no position saved
	*/
	bool GetPosition(SCR_NotificationData data, out vector position)
	{
		if (m_info.GetEditorSetPositionData() == ENotificationSetPositionData.AUTO_SET_AND_UPDATE_POSITION)
			SetPosition(data);
		
		data.GetPosition(position);
		
		if (position == vector.Zero)
			return false;
		
		return true;
	}
	
	/*!
	Set the position of the target entity the SCR_NotificationData
	\param data SCR_NotificationData to get the parameters
	*/
	void SetPosition(SCR_NotificationData data)
	{
	
	}
	
	protected bool CanSetPosition(SCR_NotificationData data)
	{
		vector position;
		data.GetPosition(position);
		
		return !((m_info.GetEditorSetPositionData() == ENotificationSetPositionData.AUTO_SET_POSITION_ONCE && position != vector.Zero) || m_info.GetEditorSetPositionData() == ENotificationSetPositionData.NEVER_AUTO_SET_POSITION);
	}
	
	//Set position data using Player ID
	protected void SetPositionDataEditablePlayer(int playerID, SCR_NotificationData data)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		SCR_EditableEntityComponent playerEntity = SCR_EditableEntityComponent.GetEditableEntity(playerManager.GetPlayerControlledEntity(playerID));
		
		if (!playerEntity) 
			return;
		
		vector position;
		
		if (!playerEntity.GetPos(position)) 
			return;
		
		data.SetPosition(position);
	}
	
	//Set position data using Editable Entity ID
	protected void SetPositionDataEditableEntity(int enditableEntityID, SCR_NotificationData data)
	{
		SCR_EditableEntityComponent targetEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(enditableEntityID));

		if (!targetEntity) return;
		
		vector position;
		if (!targetEntity.GetPos(position)) return;
		
		data.SetPosition(position);
	}
	
	/*!
	Sets the color enum related to faction if SCR_NotificationDisplayData Color is set to: 'FACTION_RELATED_FRIENDLY_IS_NEGATIVE', 'FACTION_RELATED_FRIENDLY_IS_POSITIVE', 'FACTION_RELATED_ENEMY_IS_NEGATIVE_ONLY' or 'FACTION_RELATED_FRIENDLY_IS_POSITIVE_ONLY' and there is a target to check faction of.
	\param data
	*/
	void SetFactionRelatedColor(SCR_NotificationData data)
	{

	}
	
	//Get faction related color of player target
	protected ENotificationColor GetFactionRelatedColorPlayer(int notificationPlayerID, ENotificationColor colorType)
	{
		//If not a faction set color keep current color
		if (colorType < ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			return colorType;
		
		if (!GetGame().GetPlayerController()) 
			return ENotificationColor.NEUTRAL;
		
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent) 
			return ENotificationColor.NEUTRAL;
		
		//Get factions using ID
		Faction playerSelfFaction = respawnComponent.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
		Faction notificationPlayerFaction = respawnComponent.GetPlayerFaction(notificationPlayerID);
		
		if (!playerSelfFaction || !notificationPlayerFaction) 
			return ENotificationColor.NEUTRAL;

		//Check if friendly
		if (playerSelfFaction.IsFactionFriendly(notificationPlayerFaction))
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEUTRAL;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.POSITIVE;
		}
		else
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.NEUTRAL;
		}
		
		//No color set
		return ENotificationColor.NEUTRAL;
	}
	
	//Get faction related color of entity target
	protected ENotificationColor GetFactionRelatedColorEntity(int notificationEntityID, ENotificationColor colorType)
	{	
		//If not a faction set color keep current color
		if (colorType < ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			return colorType;
		
		if (!GetGame().GetPlayerController()) return ENotificationColor.NEUTRAL;
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent) return ENotificationColor.NEUTRAL;
		
		//Get notification entity
		SCR_EditableEntityComponent notificationEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(notificationEntityID));
		if (!notificationEntity) return ENotificationColor.NEUTRAL;
		
		//Get notification entity ChimeraCharacter
		SCR_ChimeraCharacter notificationEntityChimera = SCR_ChimeraCharacter.Cast(notificationEntity.GetOwner());
		if (!notificationEntityChimera) return ENotificationColor.NEUTRAL;
		
		//Get factions
		Faction playerSelfFaction = respawnComponent.GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());	
		Faction notificationEntityFaction = notificationEntityChimera.GetFaction();
		
		if (!playerSelfFaction || !notificationEntityFaction) return ENotificationColor.NEUTRAL;
		
		//Check if friendly
		if (playerSelfFaction.IsFactionFriendly(notificationEntityFaction))
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEUTRAL;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.POSITIVE;
		}
		else
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.NEUTRAL;
		}
		
		//No color set
		return ENotificationColor.NEUTRAL;
	}	
	
	//Check if entities are friendly
	protected bool AreEntitiesFriendly(int entity1ID, bool entity1IsPlayer, int entity2ID, bool entity2IsPlayer)
	{
		Faction faction1, faction2;

		SCR_RespawnSystemComponent respawnComponent;
		
		if (entity1IsPlayer || entity2IsPlayer)
		{
			respawnComponent = SCR_RespawnSystemComponent.GetInstance();
			if (!respawnComponent) 
				return false;
		}
		
		SCR_EditableEntityComponent entity;
		SCR_ChimeraCharacter entityChimera;
		
		//Get notification entity
		
		//Get entity 1 faction
		if (entity1IsPlayer)
		{
			faction1 = respawnComponent.GetPlayerFaction(entity1ID);
		}
		else 
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entity1ID));
			if (!entity) 
				return false;
		
			//Get notification entity SCR_ChimeraCharacter
			entityChimera = SCR_ChimeraCharacter.Cast(entity.GetOwner());
			if (!entityChimera) 
				return false;
			
			faction1 = entityChimera.GetFaction();
		}
		
		//Get entity 2 faction
		if (entity2IsPlayer)
		{
			faction2 = respawnComponent.GetPlayerFaction(entity2ID);
		}
		else 
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entity2ID));
			if (!entity) 
				return false;
		
			//Get notification entity SCR_ChimeraCharacter
			entityChimera = SCR_ChimeraCharacter.Cast(entity.GetOwner());
			if (!entityChimera) 
				return false;
			
			faction2 = entityChimera.GetFaction();
		}
		
		if (!faction1 || !faction2)
			return false;
		
		return faction1.IsFactionFriendly(faction2);
	}
};