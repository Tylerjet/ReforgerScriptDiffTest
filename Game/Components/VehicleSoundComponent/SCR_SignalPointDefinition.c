[BaseContainerProps()]
class SCR_SignalPointDefinition
{
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fTimeMin;
	
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fTimeMax;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fValueMin;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fValueMax;
	
	void GenerateRandom(out SCR_SignalPoint signalPoint)
	{
		signalPoint.m_fTime = Math.RandomFloat(m_fTimeMin, m_fTimeMax);
		signalPoint.m_fValue = Math.RandomFloat(m_fValueMin, m_fValueMax);
	}
};