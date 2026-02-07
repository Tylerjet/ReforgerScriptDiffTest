class SCR_AIGetDefendWaypoint : SCR_AIGetCompletionRadius
{
	static const string USE_TURRETS_PORT = "UseTurrets";
	static const string SEARCH_TAGS_PORT = "SearchTags";
	static const string CURRENT_PRESET_PORT = "CurrentPreset";
	
	ref array<string> m_tagsArray;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut2 = {
		PORT_RADIUS,
		USE_TURRETS_PORT,
		SEARCH_TAGS_PORT,
		CURRENT_PRESET_PORT
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
		
		SCR_DefendWaypoint wp = SCR_DefendWaypoint.Cast(owner.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		m_tagsArray = wp.MakeTagsForSearch();
		
		SetVariableOut(USE_TURRETS_PORT, wp.GetUseTurrets());
		SetVariableOut(SEARCH_TAGS_PORT, m_tagsArray);
		SetVariableOut(CURRENT_PRESET_PORT, wp.GetCurrentDefendPreset());
		
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
		return "Returns defend waypoint parameters";
	}		

};