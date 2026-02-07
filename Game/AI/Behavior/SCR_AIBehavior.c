class SCR_AIBehaviorBase : SCR_AIActionBase
{
	SCR_AIUtilityComponent m_Utility;
	SCR_AIActivityBase m_RelatedGroupActivity;
	
	float m_fThreat = 0.0; // This threat value will be added to the calculated threat level in threat system
	bool m_bAllowLook = true;
	bool m_bResetLook = false;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_Utility = utility;
		m_RelatedGroupActivity = groupActivity;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	SCR_AIActivityBase GetGroupActivityContext()
	{
		return m_RelatedGroupActivity;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		if (m_bResetLook)
			m_Utility.m_LookAction.Cancel();
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
	override void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
};

class SCR_AIWaitBehavior : SCR_AIBehaviorBase
{
	override float Evaluate()
	{
		return 10;
	}

	void SCR_AIWaitBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Wait.bt";
    }
};

class SCR_AIIdleBehavior : SCR_AIBehaviorBase
{
	override float Evaluate()
	{
		return 1;
	}

	void SCR_AIIdleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		if (!utility)
			return;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Idle.bt";
	}
};

