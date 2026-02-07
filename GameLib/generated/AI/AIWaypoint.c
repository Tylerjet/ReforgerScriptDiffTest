/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup AI
* @{
*/

class AIWaypointClass: GenericEntityClass
{
};

/**
\brief	Enfusion waypoints do not specify just a point in the world, they can also
include a logic, goal, etc. to run at the reached waypoint.
*/
class AIWaypoint: GenericEntity
{
	proto external float GetCompletionRadius();
	proto external void SetCompletionRadius(float r);
	proto external EAIWaypointCompletionType GetCompletionType();
	proto external void SetCompletionType(EAIWaypointCompletionType newCompletionType);
	/**
	\brief      Tests whether the agent passes the test on being within the completion radius.
	
	\tparam     AIAgent*	Pointer to the agent that will be tested
	*/
	proto external bool IsWithinCompletionRadius(AIAgent pAgent);
};

/** @}*/
