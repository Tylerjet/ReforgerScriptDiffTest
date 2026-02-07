class SCR_AIPerformActionBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<SCR_AISmartActionComponent> m_SmartActionComponent = new SCR_BTParam<SCR_AISmartActionComponent>(SCR_AIActionTask.SMARTACTION_PORT);
		
	//------------------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIPerformActionBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, SCR_AISmartActionComponent smartActionComponent, float priority = PRIORITY_BEHAVIOR_PERFORM_ACTION)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/PerformAction.bt";
        m_eType = EAIActionType.PERFORM_ACTION;	
		m_fPriority = priority;			
		m_SmartActionComponent.Init(this, smartActionComponent);		
    }
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " using smart action " + m_SmartActionComponent.ValueToString();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_SmartActionComponent.m_Value)
			m_SmartActionComponent.m_Value.ReleaseAction();	
	}
	
	// fail action if deselected example: CoverPost -> AttackBeh removes CoverPost from planner 
	//------------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		Fail();	
	}
};
