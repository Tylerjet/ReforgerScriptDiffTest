/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsConeTwistJoint: PhysicsJoint
{
	proto external void SetAngularOnly(bool angularOnly);
	/*!
	setLimit(), a few notes:
	_softness:
	0->1, recommend ~0.8->1.
	describes % of limits where movement is free.
	beyond this softness %, the limit is gradually enforced until the "hard" (1.0) limit is reached.
	_biasFactor:
	0->1?, recommend 0.3 +/-0.3 or so.
	strength with which constraint resists zeroth order (angular, not angular velocity) limit violation.
	__relaxationFactor:
	0->1, recommend to stay near 1.
	the lower the value, the less the constraint will fight velocities which violate the angular limits.
	*/
	proto external void SetLimit(int limitIndex, float limitValue);
	proto external void SetLimits(float _swingSpan1, float _swingSpan2, float _twistSpan, float _softness, float _biasFactor, float _relaxationFactor);
};

/** @}*/
