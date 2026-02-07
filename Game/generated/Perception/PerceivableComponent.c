/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Perception
* @{
*/

class PerceivableComponentClass: GameComponentClass
{
};

class PerceivableComponent: GameComponent
{
	proto external EAIUnitType GetUnitType();
	//! Fills the provided array with the list of all aimpoints as locations in model space
	//! Returns the number of output elements.
	proto external int GetAimpoints(out notnull array<vector> outPoints);
	proto external int GetAimpointsOfType(out notnull array<ref AimPoint> outPoints, EAimPointType type);
};

/** @}*/
