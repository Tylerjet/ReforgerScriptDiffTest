class SCR_AIRemoveWaypointVehicle : AITaskScripted
{	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		//type of waypoint depends on script used in GetOut waypoint prefab
		AIWaypoint wp = AIWaypoint.Cast(owner.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			return ENodeResult.FAIL;
		}
		
		ref array<AIAgent> agents = {};
		group.GetAgents(agents);
		CompartmentAccessComponent compartmentAccess;
		BaseCompartmentSlot compartment;
		IEntity agentEntity, vehicleEntity;
		
		foreach (AIAgent agent : agents)
		{
			agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
			compartmentAccess = CompartmentAccessComponent.Cast(agentEntity.FindComponent(CompartmentAccessComponent));
			if (!compartmentAccess)
				continue;
			compartment = compartmentAccess.GetCompartment();
			if (!compartment)
				continue;
			vehicleEntity = compartment.GetOwner();
			if (vehicleEntity)
			{
				group.RemoveUsableVehicle(vehicleEntity);
				return ENodeResult.SUCCESS;	
			}	
		}
		// noone of the group is inside a vehicle, wrong use of the node?
		return ENodeResult.FAIL;
		
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Returns vehicle from waypoint";
	}		

};