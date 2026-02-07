class SCR_AIMoveBehaviorBase : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_vPosition.Init(this, pos);
		SetPriority(priority);
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
		
		// if priority comes from commmanding, lower it
		if (priorityLevel == PRIORITY_LEVEL_PLAYER)
			m_fPriorityLevel.m_Value = PRIORITY_LEVEL_NORMAL;
	}
};
// Formation behavior under Follow command (high priority)
//-----------------------------------------------------------------------------------------------------
class SCR_AIFollowInFormationBehavior : SCR_AIMoveBehaviorBase
{						
	protected float m_fTimestampEnd;			// timestamp when the priority expires
	const static float DURATION_MS = 15000;		// duration of high priority follow command (ms)

	//-----------------------------------------------------------------------------------------------------
	void SCR_AIFollowInFormationBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_ACTIVITY_FOLLOW, float priorityLevel = PRIORITY_LEVEL_PLAYER)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/KeepInFormation.bt";
		if (priorityLevel > 0)
		{
			m_bAllowLook = false;
			m_bResetLook = true;
		}

		float m_fTimestamp = GetGame().GetWorld().GetWorldTime(); // world time when constructor of behavior is called
		m_fTimestampEnd = m_fTimestamp + DURATION_MS;
	}
	
	//-----------------------------------------------------------------------------------------------------	
	override float CustomEvaluate()
	{
		// decrease follow priority over time
		if(GetGame().GetWorld().GetWorldTime() >= m_fTimestampEnd)
			m_fPriorityLevel.m_Value = PRIORITY_LEVEL_NORMAL;
			
		return GetPriority();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveIndividuallyBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParamAssignable<IEntity> m_Entity = new SCR_BTParamAssignable<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParamAssignable<float> m_Radius = new SCR_BTParamAssignable<float>(SCR_AIActionTask.RADIUS_PORT);
		
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
		return this.ToString() + " moving to " + m_vPosition.m_Value.ToString();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveAndInvestigateBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<bool> m_bIsDangerous = new SCR_BTParam<bool>(SCR_AIActionTask.IS_DANGEROUS_PORT);
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>(SCR_AIActionTask.RADIUS_PORT);
	ref SCR_BTParam<bool> m_bResetTimer = new SCR_BTParam<bool>(SCR_AIActionTask.RESET_TIMER_PORT);
	ref SCR_BTParam<float> m_fTimeOut = new SCR_BTParam<float>(SCR_AIActionTask.TIMEOUT_PORT);
	ref SCR_BTParam<float> m_fDuration = new SCR_BTParam<float>("Duration"); // How much to investigate once we have arrived
	ref SCR_BTParam<vector> m_vStartPos = new SCR_BTParam<vector>("StartPos"); // Our position when the behavior was created
	
	EAIUnitType m_eTargetUnitType;
	float m_fTimeStamp;													// world time when constructor of behavior is called
	bool m_bCanTimeout = true;											// can timeout when not executed?
	protected static const float INVESTIGATION_TIMEOUT_MS = 20000;		// how long it can take NOT to investigate before the investigation becomes obsolete in ms
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveAndInvestigateBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, float priorityLevel = PRIORITY_LEVEL_NORMAL, float radius = 10, bool isDangerous = true, EAIUnitType targetUnitType = EAIUnitType.UnitType_Infantry, float duration = 10.0)
	{
		m_bIsDangerous.Init(this, isDangerous);
		m_fRadius.Init(this, radius);
		m_bResetTimer.Init(this, false);
		//m_fTimeOut.Init(this, Math.RandomFloat(20,50));
		m_fTimeOut.Init(this, 5.0 * 60.0); // For now it's a reasonable long enough time
		m_fDuration.Init(this, Math.RandomFloat(0.8*duration, 1.2*duration));
		m_eTargetUnitType = targetUnitType;
		m_vStartPos.Init(this, vector.Zero);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveAndInvestigate.bt";
		if (m_Utility)
		{
			// marking time of creation of this move and investigate (world time)
			m_fTimeStamp = GetGame().GetWorld().GetWorldTime();
		}
		
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
		m_bCanTimeout = false;
		m_Utility.m_CombatComponent.SetExpectedEnemyType(m_eTargetUnitType);
		
		// Initialize m_vStartPos value
		if (m_vStartPos.m_Value == vector.Zero)
			m_vStartPos.m_Value = m_Utility.m_OwnerEntity.GetOrigin();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AISetInvestigateBehaviorParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIMoveAndInvestigateBehavior(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
};
