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
	//! A collider shouldn't be attached to multiple hitzones. Returns the hitzone matching those collider IDs
	//! Returns the count of elements that were filled into the array.
	proto external int GetHitZonesByColliderIDs(out notnull array<HitZone> outHitZones, notnull array<int> colliderIDs);
	proto external HitZone GetHitZone(string colliderName);
	//! Clears and fills the specified outHitZones array with all HZs.
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZones(out notnull array<HitZone> outHitZones);

	// callbacks

	// --------------------------------------------------------------------------------
	event protected bool OnRplSave(ScriptBitWriter writer) { return true; };
	event protected bool OnRplLoad(ScriptBitReader reader) { return true; };
}

/*!
\}
*/
