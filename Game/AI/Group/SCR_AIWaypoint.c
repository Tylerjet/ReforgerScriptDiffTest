class SCR_AIWaypointClass: AIWaypointClass
{
};

class SCR_AIWaypoint : AIWaypoint
{
	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
    private float m_fPriorityLevel;
	
	private ref ScriptInvoker m_OnWaypointPropertiesChanged;
	
	//------------------------------------------------------------------------------------------------
	float GetPriorityLevel()
	{
		return m_fPriorityLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnWaypointPropertiesChanged()
	{
		if (!m_OnWaypointPropertiesChanged)
			m_OnWaypointPropertiesChanged = new ScriptInvoker();
		return m_OnWaypointPropertiesChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTransformResetImpl(TransformResetParams params)
	{
		if (m_OnWaypointPropertiesChanged)
			m_OnWaypointPropertiesChanged.Invoke();
		super.OnTransformResetImpl(params);
	}
	
	//------------------------------------------------------------------------------------------------
	// Override this to create a custom waypoint state object.
	SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIWaypointState(groupUtilityComp, this);
	}
};