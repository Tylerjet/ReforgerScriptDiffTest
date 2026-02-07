class SCR_AIGetPriorityFromWaypoint : AITaskScripted
{
	static const string PORT_PRIORITY = "IsPriority";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_PRIORITY
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(owner.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(PORT_PRIORITY, wp.IsPriority());
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
		return "Returns if waypoint is priority, i.e. autonomous behavior of AI will be ignored.";
	}
};