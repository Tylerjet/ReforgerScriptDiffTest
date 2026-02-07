// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)

class SCR_AIHealBehavior : SCR_AIBehaviorBase
{
	IEntity m_EntityToHeal;
	
	SCR_AIInfoComponent m_AIInfo;
	
	ScriptedDamageManagerComponent m_DamageManager;
	
	protected const float PRIORITY_DELAY_MIN_MS = 3000.0; // Delay until action returns its max priority
	protected const float PRIORITY_DELAY_MAX_MS = 6000.0;
	
	protected float m_fTimeCreated_ms;
	protected float m_fPriorityDelay_ms;
	
	//-------------------------------------------------------------------
	
	//priority should be between normal Move and AttackMove and replaced by enum later, maybe evaluated dynamicly
	void SCR_AIHealBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, IEntity entityToHeal, bool allowHealMove, float priority = PRIORITY_BEHAVIOR_HEAL)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Heal.bt";
        m_eType = EAIActionType.HEAL;	
		m_EntityToHeal = entityToHeal;
		m_fPriority = priority;
		m_fTimeCreated_ms = GetGame().GetWorld().GetWorldTime();
		m_fPriorityDelay_ms = Math.RandomFloat(PRIORITY_DELAY_MIN_MS, PRIORITY_DELAY_MAX_MS);
		
		SCR_AIUtilityComponent aiUtilComp = SCR_AIUtilityComponent.Cast(utility);
		if (!aiUtilComp)
			return;
		IEntity owner =	aiUtilComp.m_OwnerEntity;
		if (owner)
		{
			m_DamageManager = ScriptedDamageManagerComponent.Cast(owner.FindComponent(ScriptedDamageManagerComponent));
		}
		
		if (m_DamageManager)
			m_DamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);
	}
	
	void ~SCR_AIHealBehavior()
	{
		if (m_DamageManager)
			m_DamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
	}

	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed", EAIDebugCategory.INFO, 5);
#endif
	}	
	
	protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		if (!m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
			Complete();
	}
	
	override float Evaluate()
    {
		if (GetGame().GetWorld().GetWorldTime() - m_fTimeCreated_ms > m_fPriorityDelay_ms)
			return m_fPriority;
		else
			return 0;
    }
};