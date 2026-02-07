/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

/*!
State/presence of the physics controller in physics world.
When it's created as static, only SimState::NONE or SimState::COLLISION
can be used. For dynamic ones, there is also SimState::SIMULATION for
full dynamic simulation.
*/
enum SimulationState
{
	//! body is not in simulation, nor in collision world
	NONE,
	//! body is in collision world, but it's not simulated
	COLLISION,
	//! dynamic bodies are colliding and are simulated
	SIMULATION
};

/** @}*/
