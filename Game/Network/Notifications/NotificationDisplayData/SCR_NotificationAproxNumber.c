/*!
Notification aprox number
Displays a specific text if the given number (param1) is between given min and max
SCR_NotificationData: m_iParam1 to m_iParam2 number to get the approx of
The m_iNumberDivider allows for floats as this system only works with ints. So Giving the param float value * x and dividing x here again
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationAproxNumber : SCR_NotificationDisplayData
{
	[Attribute(desc: "List of all aprox text that will show a specific text. It will check if the value is between the given min and max and if non valid are found will always use the last entry")]
	protected ref array<ref SCR_NotificationAproxDisplayContainer> m_aApproxDisplayContainers;
	
	override string GetText(SCR_NotificationData data)
	{	
		if (m_aApproxDisplayContainers.IsEmpty())
		{
			Debug.Error2("SCR_NotificationAproxNumber", string.Format("Trying to display notification '%1' but no 'm_aApproxDisplayContainers' are assigned!", typename.EnumToString(ENotification, m_NotificationKey)));
			return string.Empty;
		}
		
		int param1, param2, param3, param4, param5, param6;
		data.GetParams(param1, param2, param3, param4, param5, param6);
		
		LocalizedString display1 = GetAproxText(param1);
		LocalizedString display2 = GetAproxText(param2);
		LocalizedString display3 = GetAproxText(param3);
		LocalizedString display4 = GetAproxText(param4);
		LocalizedString display5 = GetAproxText(param5);
		LocalizedString display6 = GetAproxText(param6);
		
		data.SetNotificationTextEntries(display1, display2, display3, display4, display5, display6);
		return super.GetText(data);
	}
	
	protected LocalizedString GetAproxText(float value)
	{
		for (int i = 0; i < m_aApproxDisplayContainers.Count(); i++)
		{
			if (value >= m_aApproxDisplayContainers[i].m_fMin && value < m_aApproxDisplayContainers[i].m_fMax)
				return m_aApproxDisplayContainers[i].m_sDisplayText;
		}
		
		//~ Non was chosen so grab last in array
		return m_aApproxDisplayContainers[m_aApproxDisplayContainers.Count() -1].m_sDisplayText;
	}
};

[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sDisplayText")]
class SCR_NotificationAproxDisplayContainer
{
	[Attribute(desc: "Text shown in the notification at the param location", uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sDisplayText;
	
	[Attribute(desc: "(inclusive) if nr is greater or equal than this and less than max",)]
	float m_fMin;
	
	[Attribute(desc: "(inclusive) if nr is smaller than this and greater or equal than min",)]
	float m_fMax;
};