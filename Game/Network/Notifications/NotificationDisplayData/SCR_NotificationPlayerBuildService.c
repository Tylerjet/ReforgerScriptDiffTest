/*!
Notification Player when the service is build.
Displays a notification: %1 = Player name, %2 = Service name, %3 = base callsing
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = EditableEntityID,  m_iParam3 = BaseID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerBuildService : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{
		int playerID, entityID, providerID;
		data.GetParams(playerID, entityID, providerID);

		data.SetNotificationTextEntries(GetPlayerName(playerID), GetEditableEntityName(entityID), GetProviderCallsign(providerID));
		return super.GetText(data);
	}
	
	//------------------------------------------------------------------------------------------------
	private string GetProviderCallsign(int providerID)
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		if (!baseManager)
			return string.Empty;
		
		SCR_CampaignBase base = baseManager.FindBaseByID(providerID);
		if (!base)
			return string.Empty;
		
		return base.GetCallsignDisplayNameOnly();
	}
};