/*
===========================================
Do not modify, this script is generated
===========================================
*/

class EquipmentStorageSlot: InventoryStorageSlot
{
	//! return true if you don't want slot to handle attached item visibility changes
	protected bool OnOccludedStateChanged(bool occluded) { return false; }
	
	proto external bool CanAttachItem(IEntity item);
	proto external bool IsOccluded();
};
