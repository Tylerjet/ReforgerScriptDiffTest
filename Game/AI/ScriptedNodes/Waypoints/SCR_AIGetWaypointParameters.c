class SCR_AIGetWaypointParameters : AITaskScripted
{
	// Inputs
	static const string PORT_WAYPOINT = "Waypoint";
	
	// Outputs
	static const string PORT_POSITION = "Position";
	static const string PORT_RADIUS = "Radius";

	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIWaypoint wp = null;
		GetVariableIn(PORT_WAYPOINT, wp);
		
		if (!wp)
		{
			ClearVariable(PORT_POSITION);
			ClearVariable(PORT_RADIUS);
			return ENodeResult.FAIL;
		}
			
		SetVariableOut(PORT_POSITION, wp.GetOrigin());
		SetVariableOut(PORT_RADIUS, wp.GetCompletionRadius());
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_WAYPOINT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION,
		PORT_RADIUS
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};