[BaseContainerProps(configRoot: true)]
class SCR_ObjectBrushArrayConfig
{
	[Attribute()]
	ref array<ref ObjectBrushObjectBase> m_aObjectArray;
	
	//-----------------------------------------------------------------------
	void InitArray()
	{
		m_aObjectArray = new array<ref ObjectBrushObjectBase>();
	}
};