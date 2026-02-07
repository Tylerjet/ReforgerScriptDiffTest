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
	proto external CharacterPhysicsComponent GetCharacterPhysicsComponent();
	proto external CharacterMovementComponent GetMovementComponent();
	proto external CharacterAnimGraphComponent GetAnimGraphComponent();
	proto external CharacterSndComponent GetSoundComponent();
}

/*!
\}
*/
