class SCR_AIAttackStaticBehavior : SCR_AIAttackBehavior
{
	SCR_AIInfoComponent m_InfoComponent;
	
	//----------------------------------------------------------------------------------
	void SCR_AIAttackStaticBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity target, vector pos)
	{
		if (m_Utility)
			m_InfoComponent = m_Utility.m_AIInfo;			
		if (m_InfoComponent)
		{
			m_InfoComponent.AddUnitState(EUnitState.STATIC);
			m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/AttackStatic.bt";
			m_eType = EAIActionType.ATTACK_STATIC;       		
		}	
	}
	
	//----------------------------------------------------------------------------------
	override float Evaluate()
    {
		UpdateTargetInfo();
		if (!m_Target.m_Value && m_vPosition.m_Value == vector.Zero)
			return 0;
		
		return m_fPriority;
    }		
};