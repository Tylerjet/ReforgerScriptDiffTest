/*!
Notification Player and Faction
Displays a notification: %1 = PlayerID name, %2 = given int converted to float devided by 
SCR_NotificationData: m_iParam1 = PlayerID
Can be used for: GM (m_iParam1) set respawn time to (m_iParam2)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndNumber : SCR_NotificationPlayer
{
	[Attribute(defvalue: "1", params: "1 1000")]
	protected int m_iNumberDivider;

	override string GetText(SCR_NotificationData data)
	{	
		int playerID, param1, param2, param3, param4;
		data.GetParams(playerID, param1, param2, param3, param4);
		
		float display1 = param1;
		display1 = param1 / m_iNumberDivider;
		
		float display2 = param2;
		display2 = param2 / m_iNumberDivider;
		
		float display3 = param3;
		display3 = param3 / m_iNumberDivider;
		
		float display4 = param4;
		display4 = param4 / m_iNumberDivider;
		
		data.SetNotificationTextEntries(GetPlayerName(playerID), display1.ToString(), display2.ToString(), display3.ToString(), display4.ToString());
		return super.GetText(data);
	}
};