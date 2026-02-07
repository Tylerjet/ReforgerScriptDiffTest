/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class Physics6DOFJoint: PhysicsJoint
{
	/*!
	- free means upper < lower,
	- locked means upper == lower
	- limited means upper > lower
	- axis: first 3 are linear, next 3 are angular
	*/
	proto external void SetLinearLimits(vector linearLower, vector linearUpper);
	proto external void SetAngularLimits(vector angularLower, vector angularUpper);
	proto external void SetLimit(int axis, float lo, float hi);
};

/** @}*/
