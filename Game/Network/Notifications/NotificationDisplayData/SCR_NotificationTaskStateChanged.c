[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskStateChanged : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{		
		int taskID, factionID;
		data.GetParams(taskID, factionID);
		
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(taskID));
		if (!entity)
			return string.Empty;
		
		SCR_EditorTask task = SCR_EditorTask.Cast(entity.GetOwner());
		if (!task)
			return string.Empty;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return string.Empty;
		
		Faction faction = factionManager.GetFactionByIndex(factionID);
		if (!faction)
			return string.Empty;
	
		
		data.SetNotificationTextEntries(task.GetLocationName(), entity.GetDisplayName(), faction.GetUIInfo().GetName());		
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		int taskID, factionID;
		data.GetParams(taskID, factionID);
		
		SetPositionDataEditableEntity(taskID, data);
	}
};