/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleHelicopterSimulationClass: VehicleBaseSimulationClass
{
}

class VehicleHelicopterSimulation: VehicleBaseSimulation
{
	//! Sets throttle input
	//! \param in should be in range < 0, 1 >
	proto external void SetThrottle(float in);
	//! Starts the engine
	proto external void EngineStart();
	//! Stops the engine
	proto external void EngineStop();
	//! Returns true if engine is running
	proto external bool EngineIsOn();
}

/*!
\}
*/
