class ScriptedBaseInventoryStorageComponentClass : BaseInventoryStorageComponentClass
{

};

class ScriptedBaseInventoryStorageComponent : BaseInventoryStorageComponent
{
	override bool CanStoreItem(IEntity item, int slotID)
	{
		return true;
	}
	
	override bool CanRemoveItem(IEntity item)
	{
		return true;
	}
	
	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		return true;
	}
};