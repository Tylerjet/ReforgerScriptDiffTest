class SCR_AIWaypointClass: AIWaypointClass
{
};

class SCR_AIWaypoint : AIWaypoint
{
	[Attribute("0", UIWidgets.CheckBox, "Waypoint overrides autonomous behavior")]
    private bool m_bIsPriority;
	
	//------------------------------------------------------------------------------------------------
	bool IsPriority()
	{
		return m_bIsPriority;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPriority(bool isPriority)
	{
		m_bIsPriority = isPriority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
};