class SCR_AIDecoIsWithinRadius : DecoratorScripted
{
	protected static ref TStringArray s_aVarsIn = {
		"WaypointIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	protected override bool TestFunction(AIAgent owner)
	{
		AIWaypoint waypoint;
		if (!GetVariableIn("WaypointIn",waypoint))
			waypoint = owner.GetCurrentWaypoint();		
		if (waypoint)
			return waypoint.IsWithinCompletionRadius(owner);
		return false;
	}	

	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "Decorator that test that all AIAgents are within completion radius of waypoint, current waypoint is used if none is provided";
	}	
};