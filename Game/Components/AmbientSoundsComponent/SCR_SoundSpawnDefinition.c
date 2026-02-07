[BaseContainerProps()]
class SCR_SoundSpawnDefinition
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundType))]
	ESoundType m_eSoundType;
			
	[Attribute("1", UIWidgets.Slider, "", "0 500 1")]
	int m_iDensityLimit;
	
	[Attribute("20", UIWidgets.Slider, "", "0 100 1")]
	int m_iSpawnDist;
	
	[Attribute("15", UIWidgets.Slider, "", "0 100 1")]
	int m_iPlayDistMin;
	
	[Attribute("25", UIWidgets.Slider, "", "0 100 1")]
	int m_iPlayDistMax;
	
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundWindCurve))]
	ESoundWindCurve m_eWindModifier;
};