class SCR_AIActivityBase : SCR_AIActionBase
{
	bool m_bIsWaypointRelated;
	
	void SCR_AIActivityBase(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated)
	{
		m_bIsWaypointRelated = isWaypointRelated;
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
