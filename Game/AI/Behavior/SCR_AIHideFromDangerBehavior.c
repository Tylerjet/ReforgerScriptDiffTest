class SCR_AIHideFromDangerBehavior : SCR_AIBehaviorBase
{
    vector m_vPosition;
	float m_fDesiredDistance;

    override float Evaluate()
    {
        return 0;
    }

	void SCR_AIHideFromDangerBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, vector pos = vector.Zero, float desiredDistance = 0)
    {
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/HideFromDanger.bt";
		m_bResetLook = true;
		m_bAllowLook = false;
	   	m_eType = EAIActionType.TAKE_COVER;
		m_vPosition = pos;
		m_fDesiredDistance = desiredDistance;    
    }
};
