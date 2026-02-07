[BaseContainerProps(configRoot: true)]
class SCR_ObjectBrushArrayConfig
{
	[Attribute()]
	ref array<ref ObjectBrushObjectBase> m_aObjectArray;

	//-----------------------------------------------------------------------
	void SCR_ObjectBrushArrayConfig()
	{
		if (!m_aObjectArray)
			m_aObjectArray = {};
	}
};