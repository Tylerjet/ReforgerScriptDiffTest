class SCR_AIDefendActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Waypoint = new ref SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<bool> m_bReinit = new ref SCR_BTParam<bool>(SCR_AIActionTask.REINIT_PORT);
		
	void SCR_AIDefendActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity waypoint, float priority = PRIORITY_ACTIVITY_DEFEND)
	{
		m_Waypoint.Init(this, waypoint);
		m_bReinit.Init(this, true);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityDefend.bt";
		m_eType = EAIActionType.DEFEND;	
		m_fPriority = priority;
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " defending area of " + m_Waypoint.ToString();
	}
};
