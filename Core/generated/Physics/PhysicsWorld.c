/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsWorld
{
	private void PhysicsWorld();
	private void ~PhysicsWorld();
	
	static proto void SetInteractionLayer(notnull IEntity worldEntity, int mask1, int mask2, bool enable);
	//!Gets global gravity
	static proto vector GetGravity(notnull IEntity worldEntity);
	//!Changes global gravity
	static proto void SetGravity(notnull IEntity worldEntity, vector g);
	//!Returns current fixed time-slice.
	static proto float GetTimeSlice(notnull IEntity worldEntity);
	//!Changes fixed time-slice. Default time step is set to 1/40s per simulation frame.
	static proto void SetTimeSlice(notnull IEntity worldEntity, float timeSlice);
	static proto int GetNumDynamicBodies(notnull IEntity worldEnt);
	static proto IEntity GetDynamicBody(notnull IEntity worldEnt, int index);
};

/** @}*/
