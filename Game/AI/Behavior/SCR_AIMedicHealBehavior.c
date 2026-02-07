// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)
class SCR_AIMedicHealBehavior : SCR_AIBehaviorBase
{
	SCR_AIConfigComponent m_Config;
	
	ref SCR_BTParam<IEntity> m_EntityToHeal = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	ref SCR_AIMoveIndividuallyBehavior m_HealMove;
	
	void SCR_AIMedicHealBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, IEntity entityToHeal, bool allowHealMove, float priority = PRIORITY_BEHAVIOR_MEDIC_HEAL)
	{
       	m_EntityToHeal.Init(this, entityToHeal);
        m_eType = EAIActionType.MEDIC_HEAL;
		m_sBehaviorTree = "{990FE3889BBA5839}AI/BehaviorTrees/Chimera/Soldier/MedicHeal.bt";
		m_fPriority = priority;
		m_bAllowLook = false;
	}
	
	override void OnActionSelected()
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}
	
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit repaired", EAIDebugCategory.INFO, 5);
#endif
	}	
	
	override void OnActionFailed()
	{
		super.OnActionFailed();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Failed repair", EAIDebugCategory.INFO, 5);
#endif
	}
};
