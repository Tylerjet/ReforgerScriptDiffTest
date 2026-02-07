/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

class CharacterMovementComponentClass: PawnMovementComponentClass
{
}

class CharacterMovementComponent: PawnMovementComponent
{
	proto external vector GetVelocityWS();
	proto external vector GetVelocityMS();
	proto external vector GetAngularVelocity();
	proto external void SetRotationMode(ECharacterRotationMode mode);
	proto external void SetMovementMaxSpeed(float movementSpeed);
	proto external float GetMovementMaxSpeed();
	proto external IEntity GetFloorEntity();
	proto external vector GetFloorNormal();
}

/*!
\}
*/
