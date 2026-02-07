[BaseContainerProps()]
class SCR_SoundEventDefinition
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundName))]
	ESoundName m_eSoundName;
	
	[Attribute("0", UIWidgets.Slider, "[ms]", "-100000 100000 1")]
	int m_iCoolDown;
	
	[Attribute("500", UIWidgets.Slider, "[ms]", "100 10000 1")]
	int m_iSampleLength;
	
	[Attribute("0", UIWidgets.EditBox, "[ms], adds sequence lenght to cool down time", "")]
	bool m_bAddSequenceLength;
	
	[Attribute("1", UIWidgets.Object)]
	ref SCR_SequenceDefinition m_SequenceDefinition;
		
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundDayTimeCurve))]
	ESoundDayTimeCurve m_eDayTimeCurve;
	
	float m_fCoolDownEnd;
};