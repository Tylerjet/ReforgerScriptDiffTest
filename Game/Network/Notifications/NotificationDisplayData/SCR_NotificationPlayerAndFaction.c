/*!
Notification Player and Faction of that player
Displays a notification: %1 = PlayerID name, %2 = Faction Name of assigned faction
SCR_NotificationData: m_iParam1 = PlayerID
Can be used for: Player (m_iParam1) joined faction 'factionName'
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndFaction : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID;
		data.GetParams(playerID);
		
		if (!GetGame().GetPlayerController()) return string.Empty;
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent) return string.Empty;
		
		//Get factions using ID
		Faction playerFaction = respawnComponent.GetPlayerFaction(playerID);
		if (!playerFaction) return string.Empty;
		
		data.SetNotificationTextEntries(GetPlayerName(playerID), playerFaction.GetUIInfo().GetName());
		return super.GetText(data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int playerID;
		data.GetParams(playerID);
		data.SetFactionRelatedColor(GetFactionRelatedColorPlayer(playerID, m_info.GetNotificationColor()));
	}
};