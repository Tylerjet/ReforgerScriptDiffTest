/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Vehicle
* @{
*/

class ExtBaseCompartmentSlot: ScriptAndConfig
{
	proto external IEntity GetOwner();
	proto external BaseControllerComponent GetController();
	// returns door index with entry position that is closest to provided point (in world space)
	proto external int PickDoorIndexForPoint(vector point);
	proto external IEntity AttachedOccupant();
	proto external IEntity GetOccupant();
	proto external UIInfo GetUIInfo();
	proto external EntitySlotInfo GetPassengerPointInfo();
	proto external bool GetForceFreeLook();
	proto external int GetCompartmentMgrID();
	proto external int GetCompartmentSlotID();
	proto external CompartmentDoorInfo GetDoorInfo(int doorIndex);
	/*!
	Checks if getting in is impossible for a given entity.
	\warning Covers cases in MP where multiple players could request GetIn.
	The state is reset after getting in is finished (successfully or not).
	\param entity Entity we check the lock against
	\return True if the entity doesn't hold the lock (can't enter)
	*/
	proto external bool IsGetInLockedFor(IEntity entity);
	/*!
	Checks if getting in is locked by anybody.
	\return True if any entity holds the lock (temporarily blocking other from entering)
	*/
	proto external bool IsGetInLocked();
	proto external CompartmentUserAction GetGetInAction();
	proto external CompartmentUserAction GetGetOutAction();
	proto external CompartmentUserAction GetSwitchSeatAction();
};

/** @}*/
