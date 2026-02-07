/*!
Behavior for waiting until AI is healed by someone else.
*/

class SCR_AIHealWaitBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<IEntity> m_HealProvider = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	ScriptedDamageManagerComponent m_DamageManager;
	
	void SCR_AIHealWaitBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, IEntity healProvider)
	{
		m_HealProvider.Init(this, healProvider);
		m_fPriority = PRIORITY_BEHAVIOR_HEAL_WAIT;
		m_sBehaviorTree = "{AAB70A7FFF8BB63C}AI/BehaviorTrees/Chimera/Soldier/Behavior_HealWait.bt";
		m_eType = EAIActionType.HEAL_WAIT;
		
		
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
	
	void ~SCR_AIHealWaitBehavior()
	{
		if (m_DamageManager)
			m_DamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
		
		auto game = GetGame();
		if (game)
		{
			ScriptCallQueue q = game.GetCallqueue();
			if (q)
				q.Remove(Callback_OnTimerAfterDamageRemoved);
		}
	}
	
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed externally", EAIDebugCategory.INFO, 5);
#endif
	}
	
	override bool OnInfoMessage(SCR_AIMessageBase msg)
	{
		SCR_AIMessage_ActionFailed actionFailedMsg = SCR_AIMessage_ActionFailed.Cast(msg);
		
		if (!actionFailedMsg)
			return false;
		
		// Fail action if someone has failed to heal us
		if (actionFailedMsg.m_eActionType == EAIActionType.MEDIC_HEAL)
		{
			// Someone failed to heal us
			if (!m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
			{
				// Maybe we don't need a heal any more?
				Complete();
			}
			else
			{
				Fail();
				
				// Report to group again that we are still wounded
				ReportWoundedToGroup();
			}
		}
		
		return true;
	}
	
	protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		// Delay the logic for finishing the event:
		// If we were bandaged a few times but didn't stop the bleeding eventually, the action will fail.
		ScriptCallQueue q = GetGame().GetCallqueue();
		q.Remove(Callback_OnTimerAfterDamageRemoved);
		q.CallLater(Callback_OnTimerAfterDamageRemoved, 700, false);
	}
	
	void Callback_OnTimerAfterDamageRemoved()
	{
		if (m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
		{
			// Still bleeding, it might mean that we need more bandages.
			// Report to group again.
			AICommunicationComponent communicationComp = m_Utility.m_OwnerAgent.GetCommunicationComponent();
			AIGroup group = m_Utility.m_OwnerAgent.GetParentGroup();
			SCR_AIMessage_NeedMoreHeal msgNeedMoreHeal = new SCR_AIMessage_NeedMoreHeal();
			msgNeedMoreHeal.SetReceiver(group);
			communicationComp.RequestBroadcast(msgNeedMoreHeal, group);
			
			Complete();
		}
		else
		{
			AICommunicationComponent communicationComp = m_Utility.m_OwnerAgent.GetCommunicationComponent();
			AIAgent myAgent = m_Utility.m_OwnerAgent;
			
			// No more bleeding
			// Set stance back
			ECharacterStance threatStance = GetStanceFromThreat(m_Utility.m_ThreatSystem.GetState());
			SCR_AIOrder_Stance msgStance = SCR_AIOrder_Stance.Create(threatStance);
			communicationComp.RequestBroadcast(msgStance, myAgent);
			
			// Report to group that we were healed
			AIGroup group = m_Utility.m_OwnerAgent.GetParentGroup();
			SCR_AIMessage_IWasHealed msgHealed = new SCR_AIMessage_IWasHealed();
			msgHealed.SetReceiver(group);
			communicationComp.RequestBroadcast(msgHealed, group);
			
			Complete();
		}
	}
	
	protected void ReportWoundedToGroup()
	{
		AIAgent myAgent = AIAgent.Cast(m_Utility.GetOwner());
		AIGroup myGroup = myAgent.GetParentGroup();
		SCR_AIMessage_Wounded woundedMsg = SCR_AIMessage_Wounded.Create(m_Utility.m_OwnerEntity);
		AICommunicationComponent communicationComp = m_Utility.m_OwnerAgent.GetCommunicationComponent();
		communicationComp.RequestBroadcast(woundedMsg, myGroup);
	}
};