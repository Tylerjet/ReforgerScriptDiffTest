class SCR_AIGetCompletionRadius : AITaskScripted
{
	static const string PORT_RADIUS 	= "RadiusOut";
	static const string PORT_WAYPOINT 	= "WaypointEntity";
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity waypointEntity;
		AIWaypoint m_Waypoint;
		
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		if (!GetVariableIn(PORT_WAYPOINT, waypointEntity))
			m_Waypoint = group.GetCurrentWaypoint();
		else
			m_Waypoint = AIWaypoint.Cast(waypointEntity);
				
		if ( m_Waypoint )
		{
			SetVariableOut(PORT_RADIUS,m_Waypoint.GetCompletionRadius());
			return ENodeResult.SUCCESS;
		}				
		
		return ENodeResult.FAIL;
	}

	protected override bool VisibleInPalette()
	{
		return false;
	}	
	
	protected override string GetOnHoverDescription()
	{
		return "Gets radius of waypoint from in-port IEntity, if not specified will attempt to get current waypoint";
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_WAYPOINT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_RADIUS
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};