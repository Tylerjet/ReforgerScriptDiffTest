[BaseContainerProps(configRoot: true)]
class SCR_SupplyStoredItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
	
		return GetConsumer(GetResourceComponent(item));
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);
		
		float supplies, maxSupplies;
		GetSupplyAmounts(item, supplies, maxSupplies);
		
		return WidgetManager.Translate(GetName(), supplies, maxSupplies);
	}
}
