[BaseContainerProps()]
class SCR_SoundSpawnPreset
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundEnvironmentType))]
	ESoundEnvironmentType m_eSoundSpawnPreset;
	
	[Attribute("25", UIWidgets.EditBox, "")]
	int m_iCloseRange;
	
	[Attribute("3", UIWidgets.EditBox, "")]
	int m_iCloseLimit;
	
	[Attribute("40", UIWidgets.EditBox, "")]
	int m_iMidRange;
	
	[Attribute("4", UIWidgets.EditBox, "")]
	int m_iMidLimit;
	
	[Attribute("55", UIWidgets.EditBox, "")]
	int m_iDistRange;
	
	[Attribute("5", UIWidgets.EditBox, "")]
	int m_iDistLimit;
	
	[Attribute("1000", UIWidgets.EditBox, "[ms]")]
	int m_iDelayTimeMin;
	
	[Attribute("2000", UIWidgets.EditBox, "[ms]")]
	int m_iDelayTimeMax;
	
	float GetDelayTime()
	{
		if (m_iDelayTimeMin == m_iDelayTimeMax)
			return m_iDelayTimeMin * 0.001;
		
		return Math.RandomIntInclusive(m_iDelayTimeMin, m_iDelayTimeMax) * 0.001;
	}
};