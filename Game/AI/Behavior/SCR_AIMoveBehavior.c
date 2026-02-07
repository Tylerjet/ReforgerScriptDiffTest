class SCR_AIMoveBehaviorBase : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_vPosition.Init(this, pos);
		m_fPriority = priority;
		m_fPriorityLevel.m_Value = priorityLevel;
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveInFormationBehavior : SCR_AIMoveBehaviorBase
{
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveInFormationBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/KeepInFormation.bt";
		if (priorityLevel > 0)
		{
			m_bAllowLook = false;
			m_bResetLook = true;
		}
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveIndividuallyBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParamAssignable<IEntity> m_Entity = new SCR_BTParamAssignable<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParamAssignable<float> m_Radius = new SCR_BTParamAssignable<float>(SCR_AIActionTask.RADIUS_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	override float Evaluate()
	{
		//Print(vector.Distance(m_vPosition, m_Utility.m_vOwnerPos));
			return super.Evaluate();
	}

	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveIndividuallyBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, float priorityLevel = PRIORITY_LEVEL_NORMAL, IEntity ent = null, float radius = 1.0)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveIndividually.bt";
		m_Entity.Init(this, ent);
		if (ent)
			m_vPosition.m_Value = ent.GetOrigin();
		m_Radius.Init(this, radius);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.ValueToString();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveAndInvestigateBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<bool> m_bIsDangerous = new SCR_BTParam<bool>(SCR_AIActionTask.IS_DANGEROUS_PORT);
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>(SCR_AIActionTask.RADIUS_PORT);
	ref SCR_BTParam<bool> m_bResetTimer = new SCR_BTParam<bool>(SCR_AIActionTask.RESET_TIMER_PORT);
	ref SCR_BTParam<float> m_fTimeOut = new SCR_BTParam<float>(SCR_AIActionTask.TIMEOUT_PORT);
	
	EAIUnitType m_eTargetUnitType;
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveAndInvestigateBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, float priorityLevel = PRIORITY_LEVEL_NORMAL, float radius = 10, bool isDangerous = true, EAIUnitType targetUnitType = EAIUnitType.UnitType_Infantry)
	{
		m_bIsDangerous.Init(this, isDangerous);
		m_fRadius.Init(this, radius);
		m_bResetTimer.Init(this, false);
		m_fTimeOut.Init(this, Math.RandomFloat(20,50));
		m_eTargetUnitType = targetUnitType;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveAndInvestigate.bt";
		if (m_Utility)
			m_Utility.SetInvestigationDestination(pos);
		
		// If target is dangerous, during execution of this action we will increase our threat level
		// Aim of this is to be in alerted state through the action, so that when we encounter enemy again,
		// We are not 'surprised' by enemy and there will be no extra delay added
		if (isDangerous)
			m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		m_Utility.m_CombatComponent.SetExpectedEnemyType(m_eTargetUnitType);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		// when switching from this behavior, it becomes obsolete to continue investigating lost target
		Fail();
		m_Utility.ClearInvestigationDestination();
		super.OnActionDeselected();
	}
	
	//-----------------------------------------------------------------------------------------------------
	void OnReceivedIrrelevantInvestigation()
	{
		m_bResetTimer.m_Value = true;
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AISetInvestigateBehaviorParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIMoveAndInvestigateBehavior(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
};
