/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Physics
* @{
*/

sealed class PhysicsJoint: pointer
{
	static proto PhysicsHingeJoint CreateHinge(notnull IEntity ent1, notnull IEntity ent2, vector point1, vector axis1, vector point2, vector axis2, bool block, float breakThreshold);
	static proto PhysicsHingeJoint CreateHinge2(notnull IEntity ent1, notnull IEntity ent2, vector matrix1[4], vector matrix2[4], bool block, float breakThreshold);
	static proto PhysicsBallSocketJoint CreateBallSocket(notnull IEntity ent1, notnull IEntity ent2, vector point1, vector point2, bool block, float breakThreshold);
	static proto PhysicsFixedJoint CreateFixed(notnull IEntity ent1, notnull IEntity ent2, vector point1, vector point2, bool block, float breakThreshold);
	static proto PhysicsConeTwistJoint CreateConeTwist(notnull IEntity ent1, notnull IEntity ent2, vector matrix1[4], vector matrix2[4], bool block, float breakThreshold);
	static proto PhysicsSliderJoint CreateSlider(notnull IEntity ent1, notnull IEntity ent2, vector matrix1[4], vector matrix2[4], bool block, float breakThreshold);
	static proto Physics6DOFJoint Create6DOF(notnull IEntity ent1, notnull IEntity ent2, vector matrix1[4], vector matrix2[4], bool block, float breakThreshold);
	static proto Physics6DOFSpringJoint Create6DOFSpring(notnull IEntity ent1, notnull IEntity ent2, vector matrix1[4], vector matrix2[4], bool block, float breakThreshold);
	proto external void Destroy();
};

/** @}*/
