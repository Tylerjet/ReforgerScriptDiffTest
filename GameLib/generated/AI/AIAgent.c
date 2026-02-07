/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIAgentClass: GenericControllerClass
{
}

class AIAgent: GenericController
{
	proto external IEntity GetControlledEntity();
	proto external AIGroup GetParentGroup();
	proto external void SetLOD(int newLOD);
	proto external int GetLOD();
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
	proto external AIPathfindingComponent GetPathfindingComponent();
}

/*!
\}
*/
