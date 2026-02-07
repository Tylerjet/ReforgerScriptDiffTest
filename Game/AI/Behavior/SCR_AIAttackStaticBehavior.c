class SCR_AIAttackStaticBehavior : SCR_AIAttackBehavior
{
	SCR_AIInfoComponent m_InfoComponent;
	
	//----------------------------------------------------------------------------------
	void SCR_AIAttackStaticBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, BaseTarget target, vector pos)
	{
		if (m_Utility)
			m_InfoComponent = m_Utility.m_AIInfo;			
		if (m_InfoComponent)
		{
			m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/AttackStatic.bt";
			m_eType = EAIActionType.ATTACK_STATIC;       		
		}	
	}
};