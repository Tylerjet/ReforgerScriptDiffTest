/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup AI
* @{
*/

class AIBaseMovementComponentClass: AIComponentEntityClass
{
};

class AIBaseMovementComponent: AIComponentEntity
{
	// Script functions
	proto external void RequestMove(Managed user, string callbackFncName, vector pos, float precision);
	proto external void RequestMoveAndOrient(Managed user, string callbackFncName, vector pos, vector orientPos, float precision);
};

/** @}*/
