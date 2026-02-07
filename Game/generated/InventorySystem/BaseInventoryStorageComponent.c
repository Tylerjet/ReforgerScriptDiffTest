/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup InventorySystem
* @{
*/

class BaseInventoryStorageComponentClass: InventoryItemComponentClass
{
};

class BaseInventoryStorageComponent: InventoryItemComponent
{
	protected ESlotID m_eSlotID = ESlotID.SLOT_ANY;
	ESlotID GetSlotID()	{ return m_eSlotID; };
	
	/*!
	*IMPORTANT* Should be called upon initialization of slot instance
	provide newly created slot and desired slot ID
	*/
	proto external sealed protected void SetupSlotHooks(InventoryStorageSlot ownedSlot, int slotID);
	/*!
	*IMPORTANT* Should be called before transfering ownership of holded slot instance
	In majority of cases should be unnecessary (when storage manages creation and destruction of slots on its own)
	*/
	proto external sealed protected void ReleaseSlotHooks(InventoryStorageSlot ownedSlot);
	/*!
	Check if an item is inside this storage.
	\param item Item which meeds to be searched.
	\return Returns true if the item is inside this storage.
	*/
	proto external sealed bool Contains(IEntity item);
	/*!
	Find the item slot.
	\param item Item which meeds to be searched.
	\return Returns the slot otherwise null.
	*/
	proto external sealed InventoryStorageSlot FindItemSlot(IEntity item);
	//! Returns the number of slots in this storage.
	proto external int GetSlotsCount();
	/*!
	Return slot for specified id
	\param slotID Slot ID.
	\return Returns the slot otherwise null.
	*/
	proto external sealed InventoryStorageSlot GetSlot(int slotID);
	/*!
	Get item at slot index.
	\param index Slot index
	\return Returns the item otherwise null.
	*/
	proto external sealed IEntity Get(int index);
	// Returns all stored items in this storage
	proto external sealed int GetAll(out notnull array<IEntity> outItems);
	// Returns storage priority
	proto external sealed int GetPriority();
	proto external sealed EStoragePurpose GetPurpose();
	// Search for slot where item can be inserted
	proto external InventoryStorageSlot FindSuitableSlotForItem(IEntity item);
	//! returns amount of space occupied by attached items
	proto external float GetOccupiedSpace();
	//! does current storage serves as a compartment of other storage
	proto external bool IsCompartment();
	//! implemented for convinience - fills array with attached items including items from storage compartments (depth 0)
	proto external void GetOwnedItems(out notnull array<IEntity> outItems);
	/*!
	Fills array with attached storages up to provided hierarchy depth
	for every top level storage depth with even number will reach compartments level and odd will reach storages attached to slots (including compartment slots)
	\param outStorages - array to be filled with storages
	\param depth - at what depth level should be storages collected
	\param includeHierarchy - should storages from upper depth level be included in array
	\return true if depth was reached
	*/
	proto external bool GetOwnedStorages(out notnull array<BaseInventoryStorageComponent> outStorages, int depth, bool includeHierarchy);
	//! performs volume and item dimension validation.
	proto external bool PerformVolumeValidation(IEntity item);
	// convinience method, returns volume calculated from dimension limits
	proto external float GetMaxVolumeCapacity();
	// return dimension limits for storage
	proto external vector GetMaxDimensionCapacity();
	
	// callbacks
	
	// Will be called when item is added to slot
	event protected void OnAddedToSlot(IEntity item, int slotID);
	// Will be called when item is removed from slot
	event protected void OnRemovedFromSlot(IEntity item, int slotID);
	// Usually any slot that item can be inserted to
	event protected InventoryStorageSlot GetEmptySlotForItem(IEntity item);
	// Implemented logics for can insert here, Manager will provide slotID of -1 in case slot is irrelevant.
	event bool CanStoreItem(IEntity item, int slotID);
	// Implemented logics for can remove here,
	event bool CanRemoveItem(IEntity item);
	// Implemented logics for can replace to nextItem at slotID,
	event bool CanReplaceItem(IEntity nextItem, int slotID);
	// Should Return slots count
	event protected int GetSlotsCountScr();
	// Should Return slot for specified id
	event protected InventoryStorageSlot GetSlotScr(int slotID);
	// Called locally per instance, implement remove logics here
	event protected ref BaseInventoryTask RemoveItem(IEntity item);
	// Called locally per instance, implement insertion logics here, Manager will provide slotID of -1 in case slot is irrelevant.
	event protected ref BaseInventoryTask InsertItem(IEntity item, int slotID);
	// Will be called to estimate if storage children has to be included in preview
	event protected bool ShouldPreviewAttachedItems();
	// will be called when manager is changed, manager can be null if there is no manager in hierarchy (item drop in world)
	event protected void OnManagerChanged(InventoryStorageManagerComponent manager);
};

/** @}*/
