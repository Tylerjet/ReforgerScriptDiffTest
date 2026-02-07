[BaseContainerProps()]
class SCR_SoundSpawnDefinitionGroup
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESoundEnvironmentType))]
	ESoundEnvironmentType m_eSoundSpawnPreset;
	
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundSpawnDefinition> m_aSoundSpawnDefinition;
};