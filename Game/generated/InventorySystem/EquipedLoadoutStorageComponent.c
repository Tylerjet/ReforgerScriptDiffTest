/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class EquipedLoadoutStorageComponentClass: ScriptedBaseInventoryStorageComponentClass
{
}

class EquipedLoadoutStorageComponent: ScriptedBaseInventoryStorageComponent
{
	//! Get the first slot that satisfies the condition : "slot area type is inherited by pAreaType".
	proto external InventoryStorageSlot GetSlotFromArea(typename pAreaType);
	//! Get the first cloth entity that satisfies the condition : "slot area type is inherited by pAreaType AND slot has attached entity".
	proto external IEntity GetClothFromArea(typename pAreaType);
}

/*!
\}
*/
