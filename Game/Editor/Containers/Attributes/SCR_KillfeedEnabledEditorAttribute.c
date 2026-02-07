// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_KillfeedEnabledEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		return null;
		
		/*BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return null;
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(gamemode.FindComponent(SCR_NotificationSenderComponent));
		if (!sender)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(sender.GetShowKillfeed());*/
	}
	/*
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return;
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(gamemode.FindComponent(SCR_NotificationSenderComponent));
		if (!sender)
			return;
		
		bool value = var.GetBool();

		//Notification
		if (item)
		{
			if (value)
			{
				SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_SHOW_KILLFEED, playerID);
			}
			else 
			{
				SCR_NotificationsComponent.SendToGameMasters(ENotification.EDITOR_ATTRIBUTES_HIDE_KILLFEED, playerID);
			}
		}
		
		sender.SetShowKillfeedServer(value);
	}*/
};