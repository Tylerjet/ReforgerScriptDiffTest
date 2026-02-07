[BaseContainerProps()]
class SCR_SoundSpawnDefinition
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundType))]
	ESoundType m_eSoundType;
			
	[Attribute("1", UIWidgets.Slider, "", "0 500 1")]
	int m_iDensityLimit;
	
	[Attribute("15", UIWidgets.Slider, "", "10 100 1")]
	int m_iSpawnDistMin;
	
	[Attribute("25", UIWidgets.Slider, "", "10 100 1")]
	int m_iSpawnDistMax;
};