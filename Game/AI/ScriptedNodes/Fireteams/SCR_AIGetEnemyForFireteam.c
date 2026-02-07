class SCR_AIGetEnemyToFireteam: AITaskScripted
{
	static const string PORT_TARGET_OUT		= "TargetOut";
	static const string PORT_POSITION_OUT	= "PositionOut";
	static const string PORT_FIRETEAM_IN	= "FireteamIn";
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{	
		// TODO replace ints in trees 
		/*
		if (GetVariableType(true, PORT_FIRETEAM_IN) != EFireTeams)
		{
			NodeError(this, owner, PORT_FIRETEAM_IN + " has to be EFireTeams");
		}
		*/
		
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		
		if (GetVariableType(false, PORT_POSITION_OUT) != vector)
		{
			NodeError(this, owner, PORT_POSITION_OUT + " has to be vector");
		}
		
		if (GetVariableType(false, PORT_TARGET_OUT) != IEntity)
		{
			NodeError(this, owner, PORT_TARGET_OUT + " has to be entity");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent)
			return ENodeResult.FAIL;
		
		int teamSelection;
		GetVariableIn(PORT_FIRETEAM_IN, teamSelection);
		
		for (int i = 0, length = m_GroupUtilityComponent.m_aTargetInfos.Count(); i< length; i++)
		{
			if (m_GroupUtilityComponent.m_aTargetInfos[i].m_eFireTeamAssigned == teamSelection)
			{
				if (SCR_AIIsAlive.IsAlive(m_GroupUtilityComponent.m_aTargetInfos[i].m_TargetEntity))
				{
					SetVariableOut(PORT_TARGET_OUT, m_GroupUtilityComponent.m_aTargetInfos[i].m_TargetEntity);
					SetVariableOut(PORT_POSITION_OUT, m_GroupUtilityComponent.m_aTargetInfos[i].m_vLastSeenPosition);
					return ENodeResult.SUCCESS;
				}
			}
		}
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_FIRETEAM_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TARGET_OUT,
		PORT_POSITION_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};