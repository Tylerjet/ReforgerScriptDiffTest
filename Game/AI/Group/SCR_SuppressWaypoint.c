class SCR_SuppressWaypointClass : SCR_TimedWaypointClass
{
}

class SCR_SuppressWaypoint : SCR_TimedWaypoint
{
	[Attribute("1", UIWidgets.EditBox, desc: "Height of suppression volume above waypoint center")]
	protected float m_fSuppressionHeight;
	
	float GetSuppressionHeight()
	{
		return m_fSuppressionHeight;
	}
	
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_SuppressWaypointState(groupUtilityComp, this);
	}
}

class SCR_SuppressWaypointState : SCR_AIWaypointState
{
	override void OnSelected()
	{
		super.OnSelected();
		
		m_Utility.SetStateAllActionsOfType(SCR_AISuppressActivity, EAIActionState.FAILED);
		
		float suppressHeight = 1.0;
		SCR_SuppressWaypoint suppressWp = SCR_SuppressWaypoint.Cast(m_Waypoint);
		if (suppressWp)
			suppressHeight = suppressWp.GetSuppressionHeight();
		
		SCR_AISuppressionVolumeWaypoint volume = new SCR_AISuppressionVolumeWaypoint(vector.Zero, vector.Zero);
		volume.SetWaypoint(m_Waypoint, suppressHeight);
		
		float priorityLevel = 0;
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
			priorityLevel = wp.GetPriorityLevel();
		
		SCR_AISuppressActivity activity = new SCR_AISuppressActivity(m_Utility, m_Waypoint, volume, priorityLevel: priorityLevel);
		m_Utility.AddAction(activity);
	}
}