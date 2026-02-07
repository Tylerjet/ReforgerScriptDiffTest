/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

class CharacterMovementComponentClass: GenericComponentClass
{
}

class CharacterMovementComponent: GenericComponent
{
	proto external void AddInputVector(vector inputVectorWS);
	proto external void SetControlRotation(vector ypr);
	proto external void ConsumeInputVector();
	proto external vector GetVelocityWS();
	proto external vector GetVelocityMS();
}

/*!
\}
*/
