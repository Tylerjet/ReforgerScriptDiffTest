class SCR_AIGetEntityFromWaypoint : AITaskScripted
{
	static const string PORT_ENTITY = "Entity";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ENTITY
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
		
		IEntity entity = wp.GetEntity();
		if (!entity)
		{
			return ENodeResult.FAIL;
		}
		SetVariableOut(PORT_ENTITY,entity);
		
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
		return "Returns entity stored on waypoint of class SCR_AIEntityWaypoint";
	}		

};