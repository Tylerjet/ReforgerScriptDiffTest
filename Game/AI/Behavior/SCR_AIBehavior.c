class SCR_AIBehaviorBase : SCR_AIActionBase
{
    SCR_AIUtilityComponent m_Utility;
	SCR_AIActivityBase m_RelatedGroupActivity;
	
	bool m_bAllowLook = true;
	bool m_bResetLook = false;

	void SCR_AIBehaviorBase(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(utility);
		m_RelatedGroupActivity = groupActivity;
	}
	
	void SetCustomPriority(float priority)
	{
		m_fPriority = priority;
	}
	
	SCR_AIActivityBase GetGroupActivityContext()
	{
		return m_RelatedGroupActivity;
	}
	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		if (m_bResetLook)
			m_Utility.m_LookAction.Cancel();
	}
};

class SCR_AIWaitBehavior : SCR_AIBehaviorBase
{
	override float Evaluate()
    {
        return 10;
    }

    void SCR_AIWaitBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Wait.bt";
        m_eType = EAIActionType.WAIT;
    }
};

class SCR_AIIdleBehavior : SCR_AIBehaviorBase
{
	override float Evaluate()
    {
		return 1;
    }

    void SCR_AIIdleBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
    {
		if (!utility)
			return;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Idle.bt";
        m_eType = EAIActionType.IDLE;
    }
};

