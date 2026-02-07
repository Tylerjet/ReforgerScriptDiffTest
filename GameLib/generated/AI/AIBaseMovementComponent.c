/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIBaseMovementComponentClass: AIComponentEntityClass
{
}

class AIBaseMovementComponent: AIComponentEntity
{
	proto external bool HasCompletedRequest(bool bTakeTimeIntoAccount);
	proto external bool RequestFollowPathOfEntity(IEntity other);
	proto external AIPathfindingComponent GetPathfindingComponent();

	// callbacks

	event void OnPathSet();
}

/*!
\}
*/
