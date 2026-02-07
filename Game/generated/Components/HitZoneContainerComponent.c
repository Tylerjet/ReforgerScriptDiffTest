/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

sealed class HitZoneContainerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external HitZone GetDefaultHitZone();
	proto external HitZone GetHitZoneByColliderID(int colliderID);
	//! Clears and fills the specified outHitZones array with all HZs that are attached to colliderIDs.
	//! Returns the hitzones matching those collider IDs
	//! Returns the count of elements that were filled into the array.
	proto external int GetHitZonesByColliderIDs(out notnull array<HitZone> outHitZones, notnull array<int> colliderIDs);
	proto external HitZone GetHitZone(string colliderName);
	//! Clears and fills the specified outHitZones array with all HZs in this entity
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZones(out notnull array<HitZone> outHitZones);
	//! Clears and fills the specified outHitZones array with all HZs in this entity and its children.
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZonesInHierarchy(out notnull array<HitZone> outHitZones);
	proto external HitZoneContainerComponent GetParentHitZoneContainer();

	// callbacks

	// --------------------------------------------------------------------------------
	event protected bool OnRplSave(ScriptBitWriter writer) { return true; };
	event protected bool OnRplLoad(ScriptBitReader reader) { return true; };
}

/*!
\}
*/
