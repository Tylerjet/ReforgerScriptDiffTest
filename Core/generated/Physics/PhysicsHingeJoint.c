/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsHingeJoint: PhysicsJoint
{
	proto external void SetLimits(float low, float high, float softness, float biasFactor, float relaxationFactor);
	proto external void SetMotorTargetAngle(float angle, float dt, float maxImpulse);
	proto external void SetMotor(bool enable, float targetVelocity, float maxImpulse);
	proto external void SetAxis(vector axis);
};

/** @}*/
