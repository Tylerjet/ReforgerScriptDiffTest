/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class HelicopterControllerComponentClass: VehicleControllerComponentClass
{
}

class HelicopterControllerComponent: VehicleControllerComponent
{
	//! return true if autohover system is enabled
	proto external bool GetAutohoverEnabled();
	//! enables autohover system
	proto external void SetAutohoverEnabled(bool enabled);
	//! returns true if wheel brake is active
	proto external bool GetWheelBrake();
	//! returns true if persistent wheel brake is active
	proto external bool GetPersistentWheelBrake();
}

/*!
\}
*/
