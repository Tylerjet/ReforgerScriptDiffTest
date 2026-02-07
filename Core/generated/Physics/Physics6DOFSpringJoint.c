/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class Physics6DOFSpringJoint: Physics6DOFJoint
{
	//! when stiffness == -1 && damping == -1, spring is disabled
	proto external void SetSpring(int axis, float stiffness, float damping);
};

/** @}*/
