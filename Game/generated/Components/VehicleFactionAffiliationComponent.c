/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class VehicleFactionAffiliationComponentClass: FactionAffiliationComponentClass
{
};

class VehicleFactionAffiliationComponent: FactionAffiliationComponent
{
	event protected void OnCompartmentEntered(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move);
	event protected void OnCompartmentLeft(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move);
};

/** @}*/
