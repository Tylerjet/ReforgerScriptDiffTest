[BaseContainerProps(configRoot:true)]
class SCR_ArsenalItemDisplayConfig
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_ArsenalItemDisplayData> m_aItemDisplayData;
	
	array<ref SCR_ArsenalItemDisplayData> GetAllDisplayData()
	{
		return m_aItemDisplayData;
	}
	
	bool GetItemDisplayData(ResourceName prefab, out SCR_ArsenalItemDisplayData offset)
	{
		foreach (SCR_ArsenalItemDisplayData itemOffset : m_aItemDisplayData)
		{
			if (itemOffset.GetItemResourceName() == prefab)
			{
				offset = itemOffset;
				return true;
			}
		}
		return false;
	}
};