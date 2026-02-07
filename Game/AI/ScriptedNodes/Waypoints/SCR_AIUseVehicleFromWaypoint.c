class SCR_AIUseVehicleFromWaypoint : AITaskScripted
{
	static const string PORT_VEHICLE = "Vehicle";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(owner.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		IEntity vehicle = wp.GetEntity();
		if (!vehicle)
		{
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(PORT_VEHICLE, vehicle);
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (group)
		{
			group.AddUsableVehicle(vehicle);
		}
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
		return "Returns vehicle from SCR_AIEntityWaypoint and adds it to UsableVehicles of the group";
	}		

};