[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_ItemResourceName", true)]
class SCR_ArsenalItemDisplayData
{
	[Attribute(params: "et")]
	protected ResourceName m_ItemResourceName;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "...", category: "Physics settings")]
	protected vector m_vItemOffset;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "...", category: "Physics settings")]
	protected vector m_vItemRotation;
	
	ResourceName GetItemResourceName()
	{
		return m_ItemResourceName;
	}
	
	vector GetItemOffset()
	{
		return m_vItemOffset;
	}
	
	vector GetItemRotation()
	{
		return m_vItemRotation;
	}
	
	void SCR_ArsenalItemDisplayData()
	{
		
	}
};