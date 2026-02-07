[BaseContainerProps()]
class SCR_SoundTerrainDefinition
{
	[Attribute("0", UIWidgets.EditBox, "")]
	int m_iRangeMin;
	
	[Attribute("0", UIWidgets.EditBox, "")]
	int m_iRangeMax;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundGroup))]
	ESoundGroup m_eSoundGroup;	
};