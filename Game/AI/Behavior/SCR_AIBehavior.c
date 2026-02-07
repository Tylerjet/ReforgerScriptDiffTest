class SCR_AIBehaviorBase : SCR_AIActionBase
{
    SCR_AIUtilityComponent m_Utility;
	SCR_AIActivityBase m_RelatedGroupActivity;
	
	bool m_bAllowLook = true;
	bool m_bResetLook = false;

	void SCR_AIBehaviorBase(SCR_AIBaseUtilityComponent utility, bool prioritize)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(utility);
	}
	
	void SetCustomPriority(float priority)
	{
		m_fPriority = priority;
	}
	
	void SetGroupActivityContext(SCR_AIActivityBase activity)
	{
		m_RelatedGroupActivity = activity;
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

    void SCR_AIWaitBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Wait.bt";
        m_eType = EAIActionType.WAIT;
    }
};

class SCR_AIIdleBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	
    override float Evaluate()
    {
		return 1;
    }

    void SCR_AIIdleBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize)
    {
		m_vPosition.Init(this, vector.Zero);
		
		if (!utility)
			return;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Idle.bt";
        m_eType = EAIActionType.IDLE;
		if(m_Utility.m_OwnerEntity)
			m_vPosition.m_Value = m_Utility.m_OwnerEntity.GetOrigin();
    }
};

class SCR_AIReturnToDefaultBehavior : SCR_AIBehaviorBase
{
	ECharacterStance m_Stance;
	vector m_vPosition;
	
    override float Evaluate()
    {
		if (m_Utility.m_OwnerController.GetStance() == ECharacterStance.STAND)
			return 0;
		return 5;
    }	
	
    void SCR_AIReturnToDefaultBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/ReturnToDefault.bt";
		m_bResetLook = true;
        m_eType = EAIActionType.RETURN_TO_DEFAULT;
		m_vPosition = m_Utility.GetOrigin();
		m_Stance = m_Utility.m_OwnerController.GetStance();
    }
};