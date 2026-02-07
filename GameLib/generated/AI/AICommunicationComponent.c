/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup AI
* @{
*/

class AICommunicationComponentClass: AIComponentClass
{
};

class AICommunicationComponent: AIComponent
{
	proto AIMessage CreateMessage(typename pClass);
	proto external bool RequestBroadcast(AIMessage pMessage, AIAgent receiver = null);
	
	// callbacks
	
	event void OnReceived(AIMessage pMessage);
	event void OnFrame(IEntity owner, float timeSlice);
	event void OnInit(IEntity owner);
};

/** @}*/
