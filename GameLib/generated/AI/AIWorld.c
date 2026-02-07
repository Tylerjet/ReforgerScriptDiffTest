/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIWorldClass: GenericEntityClass
{
}

class AIWorld: GenericEntity
{
	//AIBudget
	proto external int GetCurrentNumOfCharacters();
	proto external int GetMaxNumOfCharacters();
	proto external void SetMaxNumOfCharacters(int max);
	proto external bool CanAICharacterBeAdded();
	proto external void GetAIAgents(out notnull array<AIAgent> agents);
	proto external void RequestBroadcastDangerEvent(AIDangerEvent pEvent);
	// get formation definitions
	proto external AIFormationDefinition GetFormation(string pName);
	proto external NavmeshWorldComponent GetNavmeshWorldComponent(string name);
	proto external void RequestNavmeshRebuild(vector min, vector max);
	proto external void RequestNavmeshLoad(vector pos);

	// callbacks

	event void OnDebugAgentTeleport(AIAgent agent);
	event void AddedAIAgent(AIAgent agent);
	event void RemovingAIAgent(AIAgent agent);
}

/*!
\}
*/
