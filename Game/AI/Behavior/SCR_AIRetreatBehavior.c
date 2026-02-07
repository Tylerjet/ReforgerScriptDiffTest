enum EAIRetreatBehaviorType
{
	RETREAT_FROM_ENEMY = 0,
	RETREAT_WHILE_NO_AMMO = 1,	// Retreat while we have no ammo
	TACTICAL_RETREAT = 2 // NYI
};

class SCR_AIRetreatFromCurrentEnemyBehavior : SCR_AIBehaviorBase
{
	void SCR_AIRetreatFromCurrentEnemyBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
	{
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{0D1A299D5820F18E}AI/BehaviorTrees/Chimera/Soldier/Behavior_RetreatFromCurrentEnemy.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT;
	}
};

class SCR_AIRetreatNoAmmoBehavior : SCR_AIBehaviorBase
{
	void SCR_AIRetreatNoAmmoBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
	{
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{24C37E2C2AAC43C9}AI/BehaviorTrees/Chimera/Soldier/Behavior_RetreatNoAmmo.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT;
	}
};

class SCR_AIRetreatWhileLookAtBehavior : SCR_AIBehaviorBase
{		
	ref SCR_BTParamAssignable<vector> m_Target = new SCR_BTParamAssignable<vector>("RetreatToPoint");
	ref SCR_BTParamAssignable<vector> m_LookAt = new SCR_BTParamAssignable<vector>("LookAt");
	
	bool m_TimeSet = false;
	float m_Timestamp = 0;
	const float m_MaxTime = 3;
	
	void SCR_AIRetreatWhileLookAtBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity)
	{
		vector tempPos;
		m_Target.Init(this, tempPos);
		m_LookAt.Init(this, tempPos);
		 
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{9673533977BD9950}AI/BehaviorTrees/Chimera/Soldier/RetreatWhileLookAt.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT_MELEE;
	}
	
	override float Evaluate()
    {
		if(!m_TimeSet)
		{
			m_Timestamp = GetGame().GetWorld().GetWorldTime(); 
			m_TimeSet = true;
		}
		
		//get timeslice
		float current = GetGame().GetWorld().GetWorldTime();
		
		float secondsPassed = (current - m_Timestamp)/1000;
		
		//if the timer has passed, call Fail()
		if(secondsPassed >= m_MaxTime)
		{
			Fail();
		}
		
		return m_fPriority;
    }
};



//----------------------------------------------------------------------------------------------------------------------------------------------------------
class SCR_AIRetreatFromTargetBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamRef<BaseTarget> m_RetreatFromTarget = new SCR_BTParamRef<BaseTarget>("RetreatFromTarget");
	
	void SCR_AIRetreatFromTargetBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, BaseTarget retreatFromTarget)
	{
		m_RetreatFromTarget.Init(this, retreatFromTarget);
		
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{91B8D5FDB60C1C80}AI/BehaviorTrees/Chimera/Soldier/RetreatFromTarget.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT_FROM_TARGET;
	}
}

class SCR_AIGetRetreatFromTargetBehaviorParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIRetreatFromTargetBehavior(null, false, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};
