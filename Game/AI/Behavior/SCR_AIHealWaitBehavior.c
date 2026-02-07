/*!
Behavior for waiting until AI is healed by someone else.
*/

class SCR_AIHealWaitBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<IEntity> m_HealProvider = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	ScriptedDamageManagerComponent m_DamageManager;
	
	void SCR_AIHealWaitBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity healProvider)
	{
		m_HealProvider.Init(this, healProvider);
		
		if (!utility)
			return;
		
		m_fPriority = PRIORITY_BEHAVIOR_HEAL_WAIT;
		m_sBehaviorTree = "{AAB70A7FFF8BB63C}AI/BehaviorTrees/Chimera/Soldier/Behavior_HealWait.bt";
		m_eType = EAIActionType.HEAL_WAIT;
		
		SCR_AIUtilityComponent aiUtilComp = SCR_AIUtilityComponent.Cast(utility);
		if (aiUtilComp)
		{
			m_DamageManager = aiUtilComp.m_DamageManager;
		}
		
		if (m_DamageManager)
			m_DamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);
	}
	
	void ~SCR_AIHealWaitBehavior()
	{
		if (m_DamageManager)
			m_DamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
	}
	
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed externally", EAIDebugCategory.INFO, 5);
#endif
	}	
	
	protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		// Complete if not wounded any more
		if (!m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
		{
			// Set stance back
			AIAgent myAgent = AIAgent.Cast(m_Utility.GetOwner());
			ECharacterStance threatStance = GetStanceFromThreat(m_Utility.m_ThreatSystem.GetState());
			SCR_AIOrder_Stance msg = SCR_AIOrder_Stance.Create(threatStance);
			SCR_MailboxComponent myMailbox = SCR_MailboxComponent.Cast(myAgent.FindComponent(SCR_MailboxComponent));
			myMailbox.RequestBroadcast(msg, myAgent);
			
			Complete();
		}
	}
};