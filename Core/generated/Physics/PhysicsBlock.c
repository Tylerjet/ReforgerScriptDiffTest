/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

/**
\brief Disables collisions between two entities
*/
sealed class PhysicsBlock: pointer
{
	static proto PhysicsBlock Create(notnull IEntity ent1, notnull IEntity ent2);
	proto external void Remove(notnull IEntity worldEntity);
};

/** @}*/
