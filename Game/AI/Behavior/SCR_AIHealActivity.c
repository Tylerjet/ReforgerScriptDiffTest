class SCR_AIHealActivity : SCR_AIActivityBase
{
  	ref SCR_BTParam<IEntity> m_EntityToHeal = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
		
	override float Evaluate()
    {
		return m_fPriority;
    }

    void SCR_AIHealActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity ent, float priority = PRIORITY_ACTIVITY_HEAL)
    {
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityHeal.bt";
		m_eType = EAIActionType.HEAL;
		m_fPriority = priority;
		m_EntityToHeal.Init(this, ent);
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " healing unit " + m_EntityToHeal.ValueToString();
	}
};