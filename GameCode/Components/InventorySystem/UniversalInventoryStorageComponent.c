class UniversalInventoryStorageComponentClass: BaseUniversalInventoryStorageComponentClass
{
};

// Current storage variant allows dynamic scaling of slots and handles Move/Insert/Remove operations
// it will accept any entity for insertion and will remove/add it's visibility flag when inserted/removed from storage
// see CharacterInventoryStorageComponent for example of custom storage inheritance from current class
class UniversalInventoryStorageComponent : BaseUniversalInventoryStorageComponent
{
	protected sealed ref override BaseInventoryTask RemoveItem(IEntity item);
	protected sealed ref override BaseInventoryTask InsertItem(IEntity item, int slotID);

	protected sealed override InventoryStorageSlot GetSlotScr(int slotID);
	
	protected override void OnRemovedFromSlot(IEntity item, int slotID);
	protected override void OnAddedToSlot(IEntity item, int slotID);
	
	//override InventoryStorageSlot GetEmptySlotForItem(IEntity item);
	protected override bool CanStoreItem(IEntity item, int slotID);
	protected override bool CanRemoveItem(IEntity item);
		
};