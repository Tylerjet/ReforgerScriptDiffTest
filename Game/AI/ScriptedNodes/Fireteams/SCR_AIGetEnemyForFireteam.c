class SCR_AIGetEnemyToFireteam: AITaskScripted
{
	static const string PORT_TARGET_OUT		= "TargetOut";
	static const string PORT_TARGET_INFO	= "TargetInfoOut";
	static const string PORT_FIRETEAM_IN	= "FireteamIn";
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	ref SCR_AITargetInfo m_TargetInfo;
	
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
				if (SCR_AIDamageHandling.IsAlive(m_GroupUtilityComponent.m_aTargetInfos[i].m_TargetEntity))
				{
					m_TargetInfo = new SCR_AITargetInfo(); 								// use strong reference if your node uses it!
					m_TargetInfo.CopyFrom(m_GroupUtilityComponent.m_aTargetInfos[i]); 	// copy in case someone in GroupUtilityComp changes the object it is reffering to
					SetVariableOut(PORT_TARGET_OUT, m_TargetInfo.m_TargetEntity);
					SetVariableOut(PORT_TARGET_INFO, m_TargetInfo);
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
		PORT_TARGET_INFO
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};