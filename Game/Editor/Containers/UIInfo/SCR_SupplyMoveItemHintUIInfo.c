[BaseContainerProps(configRoot: true)]
class SCR_SupplyMoveItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		float supplies;
		GetSupplyAmounts(item, supplies);
		
		return supplies > 0;
	}
}
