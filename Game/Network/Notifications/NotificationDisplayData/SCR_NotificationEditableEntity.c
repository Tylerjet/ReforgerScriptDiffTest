/*!
Notification Editable Entity Display info
Displays a notification: %1 = EditableEntity name
SCR_NotificationData: m_iParam1 = editibleEntityID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEditableEntity : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{		
		int entityID;
		data.GetParams(entityID);
		data.SetNotificationTextEntries(GetEditableEntityName(entityID));		
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int entityID;
		data.GetParams(entityID);
		
		SetPositionDataEditableEntity(entityID, data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int entityID;
		data.GetParams(entityID);
		data.SetFactionRelatedColor(GetFactionRelatedColorEntity(entityID, m_info.GetNotificationColor()));
		
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (coloredTextUiInfo)
			data.SetFactionRelatedTextColor(GetFactionRelatedColorEntity(entityID, coloredTextUiInfo.GetNotificationTextColor()));	
	}
};