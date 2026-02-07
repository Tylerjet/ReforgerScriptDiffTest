class SCR_ChimeraAIAgentClass: ChimeraAIAgentClass
{
};

class SCR_ChimeraAIAgent : ChimeraAIAgent
{
	// Current waypoint of our group
	AIWaypoint m_GroupWaypoint;
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction(IEntity entity)
	{
		if (!entity)
			return null;
		
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle)
			return vehicle.GetFaction();
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEnemy(Faction otherFaction)
	{
		if (!otherFaction)
			return false;
		
		IEntity myEntity = GetControlledEntity();
		Faction myFaction = GetFaction(myEntity);
		if (myFaction)		
			return myFaction.IsFactionEnemy(otherFaction);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEnemy(IEntity entity)
	{
		return IsEnemy(GetFaction(entity));
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEnemy(AIAgent agent)
	{
		if (agent)
			return IsEnemy(agent.GetControlledEntity());
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupWaypointChanged(AIWaypoint newWaypoint)
	{
		m_GroupWaypoint = newWaypoint;
	}
};