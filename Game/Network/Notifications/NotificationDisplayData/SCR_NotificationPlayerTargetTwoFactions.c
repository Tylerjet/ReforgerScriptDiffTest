/*!
Notification Player with target two Factions that are seperate from player
Displays a notification: %1 = Player name, %2 = First faction name, %3 = Second faction name
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = firstFactionIndex,  m_iParam3 = secondFactionIndex
Sets the position to player ID if enabled
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerTargetTwoFactions : SCR_NotificationPlayerTargetFaction
{

	override string GetText(SCR_NotificationData data)
	{
		int playerID, firstFactionIndex, secondFactionIndex;
		data.GetParams(playerID, firstFactionIndex, secondFactionIndex);
		
		string firstFactionName = GetFactionName(firstFactionIndex);
		string secondFactionName = GetFactionName(secondFactionIndex);
		
		data.SetNotificationTextEntries(GetPlayerName(playerID), firstFactionName, secondFactionName);
		return super.GetText(data);
	}
};