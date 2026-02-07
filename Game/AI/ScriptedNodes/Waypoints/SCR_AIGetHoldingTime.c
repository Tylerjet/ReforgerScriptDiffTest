class SCR_AIGetHoldingTime : AITaskScripted
{
	
	protected static ref TStringArray s_aVarsOut = {
		"HoldingTimeOut"
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if ( owner )
		{
			SCR_TimedWaypoint wp = SCR_TimedWaypoint.Cast(owner.GetCurrentWaypoint());
			if ( wp )
			{
				SetVariableOut("HoldingTimeOut",wp.GetHoldingTime());
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
		return "Gets holding time set on current timed waypoint";
	}		

};