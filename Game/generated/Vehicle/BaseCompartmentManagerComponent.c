/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Vehicle
* @{
*/

class BaseCompartmentManagerComponentClass: GameComponentClass
{
};

class BaseCompartmentManagerComponent: GameComponent
{
	//! Searches for compartment by ID
	proto external BaseCompartmentSlot FindCompartment(int compartmentID);
	//! Returns a list and count of all compartments
	proto external int GetCompartments(out notnull array<BaseCompartmentSlot> outCompartments);
};

/** @}*/
