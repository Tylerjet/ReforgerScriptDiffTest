class SCR_EquipedLoadoutStorageComponentClass: EquipedLoadoutStorageComponentClass
{
};

class SCR_EquipedLoadoutStorageComponent : EquipedLoadoutStorageComponent
{
    protected sealed override BaseInventoryTask RemoveItem(IEntity item);
	protected sealed override BaseInventoryTask InsertItem(IEntity item, int slotID);
	protected sealed override void OnManagerChanged(InventoryStorageManagerComponent manager);
	sealed override ref InventoryStorageSlot GetSlotScr(int slotID);
	sealed override ref InventoryStorageSlot GetEmptySlotForItem(IEntity item);
	sealed override bool CanStoreItem(IEntity item, int slotID);
	sealed override bool CanRemoveItem(IEntity item);
	sealed override bool CanReplaceItem(IEntity nextItem, int slotID);
};
