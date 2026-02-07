/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseVehicleClass: GameEntityClass
{
}

class BaseVehicle: GameEntity
{
	//! Read-only variable filled from code, used to animate material
	float m_fMaterialVariable = 0;

	/*!
	Sets the wreck model of the vehicle. This will change the state of the vehicle to a wrecked vehicle which means some components will be deactivated.
	\param newModel New model which should be displayed
	*/
	proto external void SetWreckModel(VObject newModel);
	//! Returns true if the vehicle is not upside down
	proto external bool IsAccessible();
}

/*!
\}
*/
