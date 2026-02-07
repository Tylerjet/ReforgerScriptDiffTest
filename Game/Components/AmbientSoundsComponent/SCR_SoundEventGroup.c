[BaseContainerProps()]
class SCR_SoundEventGroup
{				
	[Attribute("", UIWidgets.EditBox, "")]
	string  m_sSoundEventGroupName;
		
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundEventDefinition> m_aSoundEventDefinition;
	
	ref array<int> m_aAvailableEvents = new array<int>;
	
	void GetAvailableEvents(float gameTime, float timeOfDay, float timeOfDayCurveValue[])
	{
		m_aAvailableEvents.Clear();

		int size = m_aSoundEventDefinition.Count();

		for (int i = 0; i < size; i++)
		{
			// Check for correct time of the day			
			if (timeOfDayCurveValue[m_aSoundEventDefinition[i].m_eDayTimeCurve] > 0)
			{
				// Check if sound is not in cooldown
				if (m_aSoundEventDefinition[i].m_fCoolDownEnd < gameTime)
				{
					m_aAvailableEvents.Insert(i);
				}
			}
		}			
	}
	
	int GetRandomEventIdx()
	{
		int size = m_aAvailableEvents.Count();
		
		if (size == 0)
			return -1;
		else	
			return m_aAvailableEvents[Math.RandomIntInclusive(0, size - 1)];
	}
};