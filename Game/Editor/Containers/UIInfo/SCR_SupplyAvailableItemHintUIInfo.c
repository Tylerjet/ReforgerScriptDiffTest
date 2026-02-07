[BaseContainerProps(configRoot: true)]
class SCR_SupplyAvailableItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		 if (!super.CanBeShown(item, focusedSlot))
			return false;

		SCR_ResourceComponent resourceComp = GetResourceComponent(item);
		if (!resourceComp)
			return false;
		
		return resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void GetSupplyAmounts(InventoryItemComponent item, out float supplies, out float maxSupplies = -1)
	{
		supplies = 0;	
		maxSupplies = 0;
		
		if (!item)
			return;
		
		SCR_ResourceComponent resourceComp = GetResourceComponent(item);
		if (!resourceComp)
			return;
		
		SCR_ResourceConsumer consumer = resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return;
		
		supplies = consumer.GetAggregatedResourceValue();
		maxSupplies = consumer.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);
		
		float supplies;
		GetSupplyAmounts(item, supplies);
		
		return WidgetManager.Translate(GetName(), supplies);
	}
}
