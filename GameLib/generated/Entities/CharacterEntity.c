/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class CharacterEntityClass: PawnEntityClass
{
}

class CharacterEntity: PawnEntity
{
	proto external CharacterMovementComponent GetMovementComponent();
	proto external CharacterAnimGraphComponent GetAnimGraphComponent();
	proto external CharacterSndComponent GetSoundComponent();
	proto external void EnableCollisionResponse(bool state);
}

/*!
\}
*/
