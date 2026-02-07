/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Vehicle
* @{
*/

class CarControllerComponentClass: VehicleControllerComponentClass
{
};

class CarControllerComponent: VehicleControllerComponent
{
	proto external void CancelStart();
	proto external bool HasAutomaticGearbox();
	//! returns simulation component of this controllers
	proto external VehicleWheeledSimulation GetSimulation();
	//! returns true if throttle "turbo" modifier is active
	proto external bool IsThrottleTurbo();
	proto external bool GetPersistentHandBrake();
	proto external void SetPersistentHandBrake(bool newValue);
	
	// callbacks
	
	//! Gets called when the engine start routine begins (animation event).
	event void OnEngineStartBegin();
	//! Get called while engine starter is active.
	event void OnEngineStartProgress();
	//! Gets called when the engine start routine was interrupted.
	event void OnEngineStartInterrupt();
	//! Gets called when the engine start routine has successfully completed.
	event void OnEngineStartSuccess();
	//! Gets called when the engine start routine has failed.
	event void OnEngineStartFail(EVehicleEngineStartFailedReason reason);
	/*!
	Is called every time the controller wants to start the engine.
	
	\return true if the engine can start, false otherwise.
	*/
	event bool OnBeforeEngineStart();
	//! Is called every time the engine starts.
	event void OnEngineStart();
	//! Is called every time the engine stops.
	event void OnEngineStop();
	event void OnPostInit(IEntity owner);
	event void OnDelete(IEntity owner);
};

/** @}*/
