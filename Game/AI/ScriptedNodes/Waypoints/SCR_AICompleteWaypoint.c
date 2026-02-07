class SCR_AICompleteWaypoint : AITaskScripted
{
	protected static ref TStringArray s_aVarsIn = {
		"WaypointIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if ( owner )
		{
			AIWaypoint wp;
			GetVariableIn("WaypointIn",wp);
			if ( !wp )
				wp = owner.GetCurrentWaypoint();
			if ( wp )
			{
				owner.CompleteWaypoint(wp);
				return ENodeResult.SUCCESS;
			}				
		}
		return ENodeResult.FAIL;
	}

	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected override string GetOnHoverDescription()
	{
		return "Completes current waypoint";
	}		

};