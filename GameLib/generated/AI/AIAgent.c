/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup AI
* @{
*/

class AIAgentClass: GenericControllerClass
{
};

class AIAgent: GenericController
{
	proto external IEntity GetControlledEntity();
	proto external AIGroup GetParentGroup();
	proto external void SetLOD(int newLOD);
	proto external int GetLOD();
	// waypoints handling
	proto external void AddWaypoint(AIWaypoint w);
	proto external void RemoveWaypoint(AIWaypoint w);
	proto external void AddWaypointAt(AIWaypoint w, int index);
	proto external void RemoveWaypointAt(int index);
	//! Completes specified waypoint and removes it from the list of waypoints, no need to call RemoveWaypoint
	proto external void CompleteWaypoint(AIWaypoint w);
	proto external AIWaypoint GetCurrentWaypoint();
	proto external int GetWaypoints(out array<AIWaypoint> outWaypoints);
	proto external int GetDangerEventsCount();
	proto external AIDangerEvent GetDangerEvent(int index);
	proto external void ClearDangerEvents(int howMany);
	proto external AIOrder GetCurrentOrder();
	proto external void ClearOrders();
	proto external void FinishCurrentOrder();
	proto external bool HasOrders();
	proto external ActionManager GetActionManager();
	proto external AICommunicationComponent GetCommunicationComponent();
	proto external AIControlComponent GetControlComponent();
	proto external AIBaseMovementComponent GetMovementComponent();
	proto external AIBaseAimingComponent GetAimingComponent();
	
	// callbacks
	
	//! Gets called for every danger event received on that agent
	event protected void OnCurrentWaypointChanged(AIWaypoint currentWP, AIWaypoint prevWP);
	event protected void OnWaypointCompleted(AIWaypoint wp);
	event protected void OnWaypointAdded(AIWaypoint wp);
	event protected void OnWaypointRemoved(AIWaypoint wp);
};

/** @}*/
