enum EAIRetreatBehaviorType
{
	RETREAT_FROM_ENEMY = 0,
	RETREAT_WHILE_NO_AMMO = 1,	// Retreat while we have no ammo
	TACTICAL_RETREAT = 2 // NYI
};

class SCR_AIRetreatFromCurrentEnemyBehavior : SCR_AIBehaviorBase
{
	void SCR_AIRetreatFromCurrentEnemyBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize)
	{
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{0D1A299D5820F18E}AI/BehaviorTrees/Chimera/Soldier/Behavior_RetreatFromCurrentEnemy.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT;
	}
};

class SCR_AIRetreatNoAmmoBehavior : SCR_AIBehaviorBase
{
	void SCR_AIRetreatNoAmmoBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize)
	{
		m_eType = EAIActionType.RETREAT;
		m_sBehaviorTree = "{24C37E2C2AAC43C9}AI/BehaviorTrees/Chimera/Soldier/Behavior_RetreatNoAmmo.bt";
		m_fPriority = PRIORITY_BEHAVIOR_RETREAT;
	}
};