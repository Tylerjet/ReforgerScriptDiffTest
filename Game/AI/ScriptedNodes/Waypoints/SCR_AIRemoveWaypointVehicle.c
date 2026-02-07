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
		
		IEntity leaderEntity = group.GetLeaderEntity();
		
		// I use this method of geting actual vehicle instead of 
		// group.GetUsableVehicles(); because it could lead in removing all group vehicles when the node is used wrongly  
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(leaderEntity.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
		{
			// When this error ocure waypoints was most probably used incorrectly or you are traying to use this node when there is no vehicle.
			return NodeError(this, owner, "Can't find CompartmentAccessComponent.");
		}
		
		auto compartment = compartmentAccess.GetCompartment();
		if (!compartment)
		{
			// Something went wrong. In code can be caused because there is no vehicle assigned to CompartmentAccessComponent
			return NodeError(this, owner, "Can't find CompartmentSlot.");
		}
		
		auto own = compartment.GetOwner();
		
		if (!own)
		{
			return ENodeResult.FAIL;
		}
		
		group.RemoveUsableVehicle(own);

		return ENodeResult.SUCCESS;
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