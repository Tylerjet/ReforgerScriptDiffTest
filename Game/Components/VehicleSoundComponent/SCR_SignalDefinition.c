[BaseContainerProps()]
class SCR_SignalDefinition
{
	[Attribute("", UIWidgets.EditBox, "")]
	string m_sSignalName;
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_SignalPointDefinition> m_aSignalPointDefinition;
	
	// Stores current signal shape
	ref array<ref SCR_SignalPoint> m_aSignalPoint = new array<ref SCR_SignalPoint>;
	
	float m_iSignalPointCount;
	float m_iSignalIdx;
	
	//------------------------------------------------------------------------------------------------
	float GetSignalValue(float worldTime)
	{
		int i;
		
		for (i = 1; i <= m_iSignalPointCount; i++)
		{
			if (m_aSignalPoint[i].m_fTime > worldTime)
				break;
		}
		
		if (i == m_iSignalPointCount + 1)
		{
			UpdateSignalPoint(worldTime);
			i = 1;
		}
		return Math.Lerp(m_aSignalPoint[i-1].m_fValue, m_aSignalPoint[i].m_fValue, (worldTime - m_aSignalPoint[i-1].m_fTime) / (m_aSignalPoint[i].m_fTime - m_aSignalPoint[i-1].m_fTime));
	}
		
	void UpdateSignalPoint(float worldTime)
	{		
		SCR_SignalPoint firstPoint = new SCR_SignalPoint;
		
		firstPoint.m_fTime = worldTime;
		
		if (m_aSignalPoint.Count() != 0)
		{
			firstPoint.m_fValue = m_aSignalPoint[m_iSignalPointCount].m_fValue;
			m_aSignalPoint.Clear();		
		}
		
		m_aSignalPoint.Insert(firstPoint);

		for (int i = 0; i < m_iSignalPointCount; i++)
		{
			SCR_SignalPoint signalPoint = new SCR_SignalPoint;
			m_aSignalPointDefinition[i].GenerateRandom(signalPoint);
			signalPoint.m_fTime += m_aSignalPoint[i].m_fTime;			
			m_aSignalPoint.Insert(signalPoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SignalDefinition()
	{
		m_iSignalPointCount = m_aSignalPointDefinition.Count();
	}
};