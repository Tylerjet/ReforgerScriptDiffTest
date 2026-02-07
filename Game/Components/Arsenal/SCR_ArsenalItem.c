[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_ItemResourceName", true)]
class SCR_ArsenalItem
{
	[Attribute(params: "et")]
	protected ResourceName m_ItemResourceName;
	
	[Attribute("1", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemType;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemMode;
	
	protected ref Resource m_ItemResource;
	
	SCR_EArsenalItemType GetItemType()
	{
		return m_eItemType;
	}
	
	SCR_EArsenalItemMode GetItemMode()
	{
		return m_eItemMode;
	}
	
	ResourceName GetItemResourceName()
	{
		return m_ItemResourceName;
	}
	
	Resource GetItemResource()
	{
		return m_ItemResource;
	}
	
	void SCR_ArsenalItem()
	{
		if (m_ItemResourceName.IsEmpty())
		{
			return;
		}
		m_ItemResource = Resource.Load(m_ItemResourceName);
	}
};