[BaseContainerProps(configRoot:true)]
class SCR_ArsenalItemListConfig
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_ArsenalItem> m_aArsenalItems;
	
	[Attribute()]
	protected ref array<ref SCR_ArsenalItemCountConfig> m_MaxCountPerItemType;
	
	bool GetArsenalItems(out array<ref SCR_ArsenalItem> arsenalItems)
	{
		arsenalItems = m_aArsenalItems;
		return arsenalItems != null && !arsenalItems.IsEmpty();
	}
	
	bool GetItemTypeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemType)
	{
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemType = item.GetItemType();
				return true;
			}
		}
		return false;
	}
	
	bool GetItemModeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemMode)
	{
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemMode = item.GetItemMode();
				return true;
			}
		}
		return false;
	}
	
	bool CheckMaxItemCount(SCR_EArsenalItemType itemType, int currentCount)
	{
		return SCR_ArsenalItemCountConfig.CheckMaxCount(m_MaxCountPerItemType, itemType, currentCount);
	}
};