class SCR_AISetDefendWaypointPreset : AITaskScripted
{
	static const string PRESET_INDEX_PORT = "PresetIndexIn";
	static const string WAYPOINT_PORT = "WaypointIn";
		
	[Attribute("0", UIWidgets.EditBox, "Set current preset index of defend waypoint")];
	private int m_iNewPresetIndex;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT,
		PRESET_INDEX_PORT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIWaypoint waypoint;
		SCR_DefendWaypoint wp;
		int newIndex;
		
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		if (!GetVariableIn(WAYPOINT_PORT, waypoint))
		{
			waypoint = group.GetCurrentWaypoint();
		}
		
		wp = SCR_DefendWaypoint.Cast(waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Did not provide defend waypoint to set!");
		}
		
		if (!GetVariableIn(PRESET_INDEX_PORT, newIndex))
			newIndex = m_iNewPresetIndex;
		
		if (!wp.SetCurrentDefendPreset(newIndex))	
		{
			return NodeError(this, owner, "Wrong index of preset provided");
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
		return "Changes preset of defend waypoint, defined in m_DefendPresets array of defend waypoint entity.";
	}		

};