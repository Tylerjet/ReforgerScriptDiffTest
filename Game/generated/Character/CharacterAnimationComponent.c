/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterAnimationComponentClass: BaseAnimPhysComponentClass
{
}

class CharacterAnimationComponent: BaseAnimPhysComponent
{
	//! Set animation layer for third person camera
	proto external void SetAnimationLayerTPP();
	//! Set animation layer for first person camera
	proto external void SetAnimationLayerFPP();
	proto external bool SetIKTarget(string bindingName, string boneName, vector position, vector rotationRad);
	// Set an IK target plane to a plane given by specified normal vector and a dist distance from the world origin.
	proto external bool SetIKTargetPlane(string bindingName, float normalX, float normalY, float normalZ, float dist);
	/*!
	Returns the collision box of the character in a specific stance.
	\param outMin Minimum point of the collision box.
	\param outMax Maximum point of the collision box.
	\return Returns true if collision box vectors have been filled.
	*/
	proto external bool GetCollisionMinMax(ECharacterStance whichStance, out vector outMin, out vector outMax);
	proto external void GetMovementState(out CharacterMovementState movementState);
	//! heading component - AnimPhysAgent component
	proto external CharacterHeadingAnimComponent GetHeadingComponent();
	//! Character Buoyancy - AnimPhysAgent component
	proto external CharacterBuoyancyComponent GetKinematicBuoyancyComponent();
	proto external bool IsWeaponADSTag();
	proto external bool IsPrimaryTag(AnimationTagID tagID);
	proto external bool IsSecondaryTag(AnimationTagID tagID);
	//! Returns max speed for provided model direction and movement type
	proto external float GetMaxSpeed(float inputForward, float inputRight, int moveType);
	/*!
	Returns top speed for provided moveType
	To get absolute top speed -1 can be provided as a move type parameter
	*/
	proto external float GetTopSpeed(int moveType = -1, bool ignoreStance = false);
	//! Returns the current inertia speed.
	proto external vector GetInertiaSpeed();
	proto external void SetSharedVariableFloat(TAnimGraphVariable varIdx, float value, bool varHasOtherUsers);
	proto external void SetSharedVariableInt(TAnimGraphVariable varIdx, int value, bool varHasOtherUsers);
	proto external void SetSharedVariableBool(TAnimGraphVariable varIdx, bool value, bool varHasOtherUsers);
	//! command handler access
	proto external CharacterCommandHandlerComponent GetCommandHandler();
	//! Returns if Character is currently ragdolling.
	proto external bool IsRagdollActive();
	//! Adds damage effectors that will be applied to ragdoll once it is enabled
	//! Position and Direction in character's local space, force is in m/kg3, maxLifeTime is time in seconds as for how long this damage will be applicable (scales with decrease)
	proto external void AddRagdollEffectorDamage(vector posLS, vector dirLS, float force, float radius, float maxLifeTime);
}

/*!
\}
*/
