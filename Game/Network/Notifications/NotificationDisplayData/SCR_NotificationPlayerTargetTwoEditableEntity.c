/*!
Notification Player with target Editable Entity Display info
Displays a notification: %1 = Player name, %2 = First EditableEntity name, %3 = Second EditableEntity name
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = firstEditibleEntityID,  m_iParam3 = secondEditibleEntityID
Will set notification target position to first enity only
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerTargetTwoEditableEntity : SCR_NotificationDisplayData
{

	override string GetText(SCR_NotificationData data)
	{
		int playerID, firstTargetID, secondTargetID;
		data.GetParams(playerID, firstTargetID, secondTargetID);
		
		string playerName, firstEntityName, secondEntityName;
		data.GetNotificationTextEntries(playerName, firstEntityName, secondEntityName);
		if (!GetPlayerName(playerID, playerName) || !GetEditableEntityName(firstTargetID, firstEntityName) || !GetEditableEntityName(secondTargetID, secondEntityName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, firstEntityName, secondEntityName);
		return super.GetText(data);
	}
	
	//~ Sets position to first target
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int playerID, firstTargetID;
		data.GetParams(playerID, firstTargetID);
		SetPositionDataEditableEntity(firstTargetID, data);
	}
};