class SCR_AISetDefendWaypointPreset : AITaskScripted
{
	static const string PRESET_INDEX_PORT = "PresetIndexIn";
	static const string WAYPOINT_PORT = "WaypointEntityIn";
		
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
		IEntity waypointEntity;
		SCR_DefendWaypoint wp;
		int newIndex;
		
		if (!GetVariableIn(WAYPOINT_PORT, waypointEntity))
		{
			waypointEntity = owner.GetCurrentWaypoint();			
		}
		
		wp = SCR_DefendWaypoint.Cast(waypointEntity);
		if (!wp)
		{
			NodeError(this, owner, "Did not provide defend waypoint to set!");
			return ENodeResult.FAIL;
		}
		
		if (!GetVariableIn(PRESET_INDEX_PORT, newIndex))
			newIndex = m_iNewPresetIndex;
		
		if (!wp.SetCurrentDefendPreset(newIndex))	
		{
			NodeError(this, owner, "Wrong index of preset provided");
			return ENodeResult.FAIL;
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