class SCR_AIWaypointClass: AIWaypointClass
{
};

class SCR_AIWaypoint : AIWaypoint
{
	[Attribute("0", UIWidgets.CheckBox, "Waypoint overrides autonomous behavior")]
    private bool m_bIsPriority;
	
	bool IsPriority()
	{
		return m_bIsPriority;
	}
	
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
};