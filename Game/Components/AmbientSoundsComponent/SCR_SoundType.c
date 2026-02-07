[BaseContainerProps()]
class SCR_SoundType
{
	[Attribute("", UIWidgets.EditBox, "")]
	string  m_sSoundEvent;
	
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundEventGroup> m_aSoundEventGroup;	
};