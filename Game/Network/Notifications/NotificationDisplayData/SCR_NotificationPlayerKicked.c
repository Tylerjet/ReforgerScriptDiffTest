/*!
Notification Player kicked
Displays a notification: %1 = PlayerID name, %2 = kick reason
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = kick reason
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerKicked : SCR_NotificationPlayer
{	
	[Attribute()]
	protected ref SCR_ConfigurableDialogUiPresets m_PlayerKickReasonsConfig;
	
	[Attribute(desc: "In order to get the correct kick reason from SCR_ConfigurableDialogUiPresets it will need to have the group from which it gets the reason (Make sure to add the final '_' if it has any)")]
	protected ref array<string> m_sKickReasonGroups;
	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, reason;
		data.GetParams(playerID, reason);
		
		string kickReasonText = "UNKNOWN";
		SCR_ConfigurableDialogUiPreset preset;
		
		if (m_PlayerKickReasonsConfig)
		{
			foreach (string group: m_sKickReasonGroups)
			{
				//~ Combines the m_sKickReasonGroups and the kick reason converted to string
				preset = m_PlayerKickReasonsConfig.FindPreset(group + typename.EnumToString(SCR_PlayerManagerKickReason, reason));
				
				if (preset)
				{
					kickReasonText = preset.m_sMessage;
					break;
				}
			}
		}
		
		if (!m_PlayerKickReasonsConfig)
			Print("SCR_NotificationPlayerKicked does not have the m_PlayerKickReasonsConfig assigned with a SCR_ConfigurableDialogUiPresets config prefab!", LogLevel.ERROR);
		else if (!preset)
			Print(string.Format("SCR_NotificationPlayerKicked could not find preset for kick reason %1!", typename.EnumToString(PlayerManagerKickReason, reason)), LogLevel.ERROR);

		data.SetNotificationTextEntries(GetPlayerName(playerID), kickReasonText);
		return super.GetText(data);
	}	
};