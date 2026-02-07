class SCR_AICombatMoveGroupBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<vector> m_vNextCoverPos = new ref SCR_BTParam<vector>(SCR_AIActionTask.NEXTCOVERPOSITION_PORT);
	ref SCR_BTParamRef<BaseTarget> m_Target = new ref SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT); // needed to check if one should look around when in combat move	
	
	//--------------------------------------------------------------------------------------------------------------
	override void Complete()
	{
		super.Complete();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}
	
	//--------------------------------------------------------------------------------------------------------------
	override void Fail()
	{
		super.Fail();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}

	//--------------------------------------------------------------------------------------------------------------
	void SCR_AICombatMoveGroupBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_COMBAT_MOVE_GROUP, IEntity target = null)
	{
		if (!utility)
		{
			BaseTarget _tgt; 
			m_Target.Init(this, _tgt);
			m_vNextCoverPos.Init(this, vector.Zero);
			return;
		}
		
		SCR_AIUtilityComponent util = SCR_AIUtilityComponent.Cast(m_Utility);
		if (target && util)
		{
			util.m_CombatComponent.FindTargetByEntity(target);
			m_Target.Init(this, util.m_CombatComponent.FindTargetByEntity(target));
		}
		m_vNextCoverPos.Init(this, vector.Zero);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/CombatMove.bt";
       	m_fPriority = priority;
		m_eType = EAIActionType.MOVE_COMBAT_GROUP;
		m_bUniqueInActionQueue = false;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}
};