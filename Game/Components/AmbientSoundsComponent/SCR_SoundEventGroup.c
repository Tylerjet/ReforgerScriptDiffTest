[BaseContainerProps()]
class SCR_SoundEventGroup
{				
	[Attribute("", UIWidgets.EditBox, "")]
	string  m_sSoundEventGroupName;
		
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundEventDefinition> m_aSoundEventDefinition;
	
	ref array<int> m_aAvailableEvents = new array<int>;
	
	void GetAvailableEvents(float gameTime, float timeOfDay)
	{
		m_aAvailableEvents.Clear();

		int size = m_aSoundEventDefinition.Count();

		for (int i = 0; i < size; i++)
		{			
			if (Math3D.Curve(ECurveType.CurveProperty2D, timeOfDay, m_aSoundEventDefinition[i].m_DayTimeFactor)[1] > 0)
			{
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