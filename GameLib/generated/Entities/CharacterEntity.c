/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class CharacterEntityClass: GenericEntityClass
{
}

class CharacterEntity: GenericEntity
{
	proto external void Teleport(vector transform[4]);
	proto external CharacterMovement GetCurrentMovement();
	proto external CharacterStance GetCurrentStance();
	proto external ActionManager GetActionManager();
}

/*!
\}
*/
