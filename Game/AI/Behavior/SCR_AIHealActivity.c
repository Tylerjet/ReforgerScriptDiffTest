class SCR_AIHealActivity : SCR_AIActivityBase
{
	const string GROUP_MEMBERS_EXCLUDE_PORT = "AgentsExclude";
	const string MEDIC_PORT = "MedicEntity";
	
	const int TIMEOUT_FAILED = 5*60*1000;
	
  	ref SCR_BTParam<IEntity> m_EntityToHeal = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	// Selected medic. Assigned from behavior tree.
	ref SCR_BTParam<IEntity> m_MedicEntity = new SCR_BTParam<IEntity>(MEDIC_PORT);
	
	// BTParam can't hold a ref to object, so we hold a ref to it ourselves through a member variable
	ref array<AIAgent> m_aMedicsExclude = {};
	ref SCR_BTParam<array<AIAgent>> m_aMedicsExcludeParam = new SCR_BTParam<array<AIAgent>>(GROUP_MEMBERS_EXCLUDE_PORT);
	
	protected float m_fTimeCreated_world; // World time when this was created
	
	protected float m_fTimeCheckConditions_world;
	
	override float Evaluate()
    {
		float worldTime = GetGame().GetWorld().GetWorldTime();
		
		if (worldTime - m_fTimeCheckConditions_world > 2500)
		{
			// Fail if target is null or destroyed
			if (!SCR_AIIsAlive.IsAlive(m_EntityToHeal.m_Value))
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Target entity is null or destroyed, action failed.");
				#endif
				
				Fail();
				return 0;
			}
				
			
			// Replan if medic is null or destroyed
			if (!SCR_AIIsAlive.IsAlive(m_MedicEntity.m_Value))
			{
				SetSuspended(false);
			}
			
			m_fTimeCheckConditions_world = worldTime;
		}
		
		// Fail on timeout
		if (worldTime - m_fTimeCreated_world > TIMEOUT_FAILED)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Timeout, action failed.");
			#endif
			
			Fail();
			return 0;
		}
		
		
		
		return m_fPriority;
    }

    void SCR_AIHealActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity ent, float priority = PRIORITY_ACTIVITY_HEAL)
    {
        m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityHeal.bt";
		m_eType = EAIActionType.HEAL;
		m_fPriority = priority;
		m_EntityToHeal.Init(this, ent);
		m_aMedicsExcludeParam.Init(this, m_aMedicsExclude);
		IEntity medic = null;
		m_MedicEntity.Init(this, medic); // Must use a variable, otherwise null directly doesn't work with templates.
		
		auto game = GetGame();
		if (game)
		{
			m_fTimeCreated_world = game.GetWorld().GetWorldTime();
		}
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " healing unit " + m_EntityToHeal.ValueToString();
	}
	
	override bool OnInfoMessage(SCR_AIMessageBase msg)
	{
		SCR_AIMessage_HealFailed healFailed = SCR_AIMessage_HealFailed.Cast(msg);
		
		if (healFailed)
		{
			// Did someone fail to heal our entity?
			if (healFailed.m_TargetEntity != m_EntityToHeal.m_Value)
				return false;
			
			// Don't pick this medic any more
			m_aMedicsExclude.Insert(healFailed.GetSender());
			
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("Medic %1 has failed the action. MedicsExclude: %2", healFailed.GetSender(), m_aMedicsExclude));
			#endif
			
			// Request re-run of this activity
			SetSuspended(false);
			return true;
		}
		
		SCR_AIMessage_IWasHealed wasHealed = SCR_AIMessage_IWasHealed.Cast(msg);
		if (wasHealed)
		{
			AIAgent senderAgent = wasHealed.GetSender();
			if (!senderAgent)
				return false;
			
			IEntity senderEntity = senderAgent.GetControlledEntity();
			if (!senderEntity)
				return false;
			
			if (senderEntity == m_EntityToHeal.m_Value)
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Target is healed, activity complete.");
				#endif
				
				// Send message to medic, in case the wounded unit was healed by smb else
				if (m_MedicEntity.m_Value)
				{
					AIControlComponent controlComp = AIControlComponent.Cast(m_MedicEntity.m_Value.FindComponent(AIControlComponent));
					
					if (controlComp)
					{
						AIAgent medicAgent = controlComp.GetAIAgent();
						if (medicAgent)
						{
							SCR_AIMessage_Cancel msgCancel = new SCR_AIMessage_Cancel();
							msgCancel.m_RelatedGroupActivity = this;
							m_UtilityBase.SendMessage(msgCancel, medicAgent);
						}
					}
				}
					
				// The target has been healed, we are done
				Complete();
				return true;
			}
		}
		
		SCR_AIMessage_NeedMoreHeal msgNeedMoreHeal = SCR_AIMessage_NeedMoreHeal.Cast(msg);
		if (msgNeedMoreHeal)
		{
			// The wounded soldier needs more bandage, run again
			SetSuspended(false);
		}
		
		return false;
	}
};

class SCR_AIGetHealActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIHealActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};

class SCR_AISetHealActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIHealActivity(null, false, false, null)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
};