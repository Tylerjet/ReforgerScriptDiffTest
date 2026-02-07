class SCR_AIActivityBase : SCR_AIActionBase
{
	ref SCR_BTParam<bool> m_bIsWaypointRelated = new SCR_BTParam<bool>(SCR_AIActionTask.WAYPOINT_RELATED_PORT);	
	
	void SCR_AIActivityBase(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated)
	{
		m_bIsWaypointRelated.Init(this, isWaypointRelated);
	}
};

class SCR_AIIdleActivity : SCR_AIActivityBase
{
    override float Evaluate()
    {
		return 1;
    }

    void SCR_AIIdleActivity(SCR_AIBaseUtilityComponent utility,  bool prioritize, bool isWaypointRelated)
    {
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityIdle.bt";
		m_eType = EAIActionType.IDLE;
    }
};
