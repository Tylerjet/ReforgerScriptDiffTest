/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components\InventorySystem
* @{
*/

class InventoryItemComponent: GameComponent
{
	ref ScriptInvoker<bool> m_OnLockedStateChangedInvoker = new ScriptInvoker<bool>();
	private void OnLockedStateChanged(bool nowLocked)
	{
		if (m_OnLockedStateChangedInvoker)
		{
			m_OnLockedStateChangedInvoker.Invoke(nowLocked);
		}
	}
	
	//! Returns Entity owner of current component instance
	proto external IEntity GetOwner();
	proto external bool IsLocked();
	// Get slot where item is located (returns null if none)
	proto external InventoryStorageSlot GetParentSlot();
	proto external ItemAttributeCollection GetAttributes();
	//! Hide owner entity
	proto external void HideOwner();
	//! Show owner entity
	proto external void ShowOwner();
	//! Disable owners physicall interactions
	proto external void DisablePhysics();
	//! Enable owners physicall interactions
	proto external void EnablePhysics();
	//! Enable/Disabel entity active state
	proto external void ActivateOwner(bool active);
	proto external float GetTotalWeight();
	//! Implemented for convinience, returns volume from ItemPhysicalAttributes objects
	proto external float GetVolume();
	//! Returns UI info of this item
	proto external UIInfo GetUIInfo();
	/*!
	Convinience method
	Finds first occurance of the coresponding attribute data object in owned PrefabData AttributeCollection.
	\param typeName type of the component
	*/
	proto external BaseItemAttributeData FindAttribute(typename typeName);
	//! Creates preview entity in the provided world
	proto external IEntity CreatePreviewEntity(BaseWorld world, int camera);
};

/** @}*/
