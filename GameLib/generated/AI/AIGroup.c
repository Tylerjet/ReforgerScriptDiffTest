/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup AI
* @{
*/

class AIGroupClass: AIAgentClass
{
};

class AIGroup: AIAgent
{
	proto external void AddAgent(AIAgent pAgent);
	proto external void RemoveAgent(AIAgent pAgent);
	proto external int GetAgents(notnull out array<AIAgent> outAgents);
	proto external int GetAgentsCount();
	proto external AIAgent GetLeaderAgent();
	proto external IEntity GetLeaderEntity();
	proto external AIFormationDefinition GetFormation();
	
	// callbacks
	
	event void OnEmpty();
	event void OnAgentAdded(AIAgent child);
	event void OnAgentRemoved(AIAgent child);
	event void OnLeaderChanged(AIAgent currentLeader, AIAgent prevLeader);
};

/** @}*/
