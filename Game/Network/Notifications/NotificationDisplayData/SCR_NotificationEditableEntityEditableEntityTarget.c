/*!
Notification Editable Entity with target Editable Entity Display info
Displays a notification: %1 = EditableEntity name, %2 = target EditableEntity name
SCR_NotificationData: m_iParam1 = editibleEntityID, m_iParam2 = TargetEditibleEntityID
Can be used for AI (m_iParam1) Killing other AI (m_iParam2)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEditableEntityEditableEntityTarget : SCR_NotificationPlayerTargetEditableEntity
{
	override string GetText(SCR_NotificationData data)
	{
		int entityID, targetID;
		data.GetParams(entityID, targetID);		
		data.SetNotificationTextEntries(GetEditableEntityName(entityID),GetEditableEntityName(targetID));
		return super.GetText(data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int entityID, targetEntityID;
		data.GetParams(entityID, targetEntityID);
		data.SetFactionRelatedColor(GetFactionRelatedColorEntity(targetEntityID, m_info.GetNotificationColor()));
		
		//Split notification sets faction colors of left and right texts taking player ids, Text color var set the faction color of the target entity
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (splitNotificationUIInfo)
		{
			ENotificationColor leftColor, rightColor;
			
			rightColor = GetFactionRelatedColorEntity(targetEntityID, splitNotificationUIInfo.GetRightTextColor());
			
			//Check if left color should be the same as right color
			if (splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly() && AreEntitiesFriendly(entityID, false, targetEntityID, false))
				leftColor = rightColor;
			else 
				leftColor = GetFactionRelatedColorEntity(entityID, splitNotificationUIInfo.GetLeftTextColor());
			
			data.SetSplitFactionRelatedColor(leftColor, rightColor);
		}
		else if (coloredTextUiInfo)
		{
			data.SetFactionRelatedTextColor(GetFactionRelatedColorEntity(targetEntityID, coloredTextUiInfo.GetNotificationTextColor()));
		}
				
	}
};