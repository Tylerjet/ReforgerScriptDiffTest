class SCR_AICombatMoveGroupBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<vector> m_vNextCoverPos = new ref SCR_BTParam<vector>(SCR_AIActionTask.NEXTCOVERPOSITION_PORT);
	ref SCR_BTParam<IEntity> m_Target = new ref SCR_BTParam<IEntity>(SCR_AIActionTask.TARGET_PORT); // needed to check if one should look around when in combat move
	bool m_bInCover;
	

	override void Complete()
	{
		super.Complete();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}
	
	override void Fail()
	{
		super.Fail();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}

	
	void SCR_AICombatMoveGroupBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float priority = PRIORITY_BEHAVIOR_COMBAT_MOVE_GROUP, IEntity target = null)
	{
		m_Target.Init(this, target);
		m_vNextCoverPos.Init(this, vector.Zero);
		
		if (!utility)
			return;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/CombatMove.bt";
        m_eType = EAIActionType.MOVE_COMBAT;
		m_fPriority = priority;
		m_eType = EAIActionType.MOVE_COMBAT_GROUP;
		m_bUniqueInActionQueue = false;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}
	
	protected static const float DISTANCE_MAX = 32; // Distance to next cover when threat is 0
	protected static const float DISTANCE_MIN = 16; // Minimal distance when movement is allowed
	private static const float NEAR_PROXIMITY = 6;
	
	protected const float m_StopDistance = 20 + Math.RandomFloat(0, 10);
};