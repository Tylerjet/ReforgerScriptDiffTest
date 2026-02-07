/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseCompartmentManagerComponentClass: GameComponentClass
{
}

class BaseCompartmentManagerComponent: GameComponent
{
	proto external bool AreDoorOpen(int doorIndex);
	proto external bool AreDoorFake(int doorIndex);
	proto external IEntity GetDoorUser(int doorIndex);
	proto external IEntity IsGetInAndOutBlockedByDoorUser(int doorIndex);
	proto external IEntity GetOwner();
	//! Searches for compartment by ID of slot and compartment manager.
	proto external BaseCompartmentSlot FindCompartment(int compartmentID, int mgrId = -1);
	//! Returns a list and count of all compartments
	proto external int GetCompartments(out notnull array<BaseCompartmentSlot> outCompartments);
	proto external CompartmentDoorInfo GetDoorInfo(int doorIndex);
}

/*!
\}
*/
