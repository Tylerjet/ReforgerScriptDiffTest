class SCR_AIGetInNearestWaypoint : SCR_AIGetCompletionRadius
{
	static const string PORT_WAYPOINT_PARAMS = "WaypointParams";	
	static const string PORT_ORIGIN 	= "Origin";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut2 = {
		PORT_ORIGIN,
		PORT_RADIUS,
		PORT_WAYPOINT_PARAMS
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut2;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (ENodeResult.FAIL == super.EOnTaskSimulate(owner, dt))
			return ENodeResult.FAIL;
		
		SCR_BoardingWaypoint wp = SCR_BoardingWaypoint.Cast(owner.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(PORT_WAYPOINT_PARAMS,wp.GetAllowance());
		SetVariableOut(PORT_ORIGIN,wp.GetOrigin());
		
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
		return "Returns get in nearest waypoint parameters";
	}		

};