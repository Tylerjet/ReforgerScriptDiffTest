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
	//GetAimingComponent() returns nullptr as base AIAgent doesn't provide aiming
	proto external AIBaseAimingComponent GetAimingComponent();
	// enabling, disabling, switching to player control on controlComponent
	proto external void ActivateAI();
	proto external void DeactivateAI();
	proto external bool IsAIActivated();
}

/*!
\}
*/
