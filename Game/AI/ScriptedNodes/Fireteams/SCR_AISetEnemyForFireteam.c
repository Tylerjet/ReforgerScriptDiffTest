class SCR_AISetEnemyToFireteam: AITaskScripted
{
	static const string PORT_TARGET_INFO_IN			= "TargetInfoIn";
	static const string PORT_FIRETEAM_OUT			= "FireteamOut";
	static const string PORT_ACT_INDIVIDUALLY_OUT	= "ActIndividuallyOut";
	static const int NUMBER_OF_FIRE_TEAMS_PER_GROUP	= 5;
	
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	// TODO came up with more general solution for fireteams, right now there are fixed size of 4
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent)
			return ENodeResult.FAIL;
		
		array<IEntity> targetEntities = m_GroupUtilityComponent.m_aTargetEntities;
		array<ref SCR_AITargetInfo> targetInfos = m_GroupUtilityComponent.m_aTargetInfos;
		int numberOfEnemies = targetInfos.Count();
		int numberOfGroupMembers = m_GroupUtilityComponent.m_aListOfAIInfo.Count();
		int enemyCountPerFireTeam[NUMBER_OF_FIRE_TEAMS_PER_GROUP];
		int indexOfFireTeam;
		SCR_AITargetInfo targetInfo;
		
		if (!GetVariableIn(PORT_TARGET_INFO_IN,targetInfo))
			return ENodeResult.FAIL;
		
		int targetIndex = targetEntities.Find(targetInfo.m_TargetEntity);
		if (targetIndex < 0)
			return ENodeResult.FAIL;
		
		if (numberOfEnemies >= numberOfGroupMembers) 
		{
			ClearVariable(PORT_FIRETEAM_OUT);
			SetVariableOut(PORT_ACT_INDIVIDUALLY_OUT,true);
			return ENodeResult.SUCCESS;
		}
		// how many enemies are assigned in each fireteam?
		for (int i = 0; i< numberOfEnemies; i++)
		{
			enemyCountPerFireTeam[targetInfos[i].m_eFireTeamAssigned] = enemyCountPerFireTeam[targetInfos[i].m_eFireTeamAssigned] + 1;			
		}
		// unused fireteams dont have a count, first index 0 is NO fireteam
		for (int i = m_GroupUtilityComponent.GetNumberOfFireTeams() + 1; i < NUMBER_OF_FIRE_TEAMS_PER_GROUP; i++)
		{	
			enemyCountPerFireTeam[i] = -1;
		}	
		// finding fire team with least number of assigned enemies
		
		for (int i = 0; i< numberOfEnemies; i++)
		{
			if (targetInfos[i].m_eFireTeamAssigned == EFireTeams.NONE)
			{
				indexOfFireTeam = FindLeastUsedFireTeam(enemyCountPerFireTeam);
				targetInfos[i].m_eFireTeamAssigned = indexOfFireTeam;
				enemyCountPerFireTeam[EFireTeams.NONE] = enemyCountPerFireTeam[EFireTeams.NONE] - 1;
				enemyCountPerFireTeam[indexOfFireTeam] = enemyCountPerFireTeam[indexOfFireTeam] + 1;
			}	
		}	 
		SetVariableOut(PORT_FIRETEAM_OUT, targetInfos[targetIndex].m_eFireTeamAssigned);
		SetVariableOut(PORT_ACT_INDIVIDUALLY_OUT, false);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	int FindLeastUsedFireTeam(int fireTeamsCount[NUMBER_OF_FIRE_TEAMS_PER_GROUP])
	{
		int minValue = int.MAX, fireTeamIndex;
		
		for (int i = 1; i< NUMBER_OF_FIRE_TEAMS_PER_GROUP; i++)
		{
			if (fireTeamsCount[i] < minValue && fireTeamsCount[i] > -1)
			{
				minValue = fireTeamsCount[i];
				fireTeamIndex = i;
			} 		
		}
		return fireTeamIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_FIRETEAM_OUT,
		PORT_ACT_INDIVIDUALLY_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_TARGET_INFO_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		if (!m_GroupUtilityComponent)
			return "Error: no utility component";
		
		string str;
		
		string strGroupMemberCount = string.Format("Group members: %1\n", m_GroupUtilityComponent.m_aListOfAIInfo.Count());
		
		string strEnemiesCount = string.Format("Enemies: %1\n", m_GroupUtilityComponent.m_aTargetInfos.Count());
		
		string strEnemiesToFireteams = "Enemies -> Fireteams:\n";
		foreach (int index, SCR_AITargetInfo info : m_GroupUtilityComponent.m_aTargetInfos)
			strEnemiesToFireteams = strEnemiesToFireteams + string.Format("  %1: %2 %3\n", index, info.m_eFireTeamAssigned, typename.EnumToString(EFireTeams, info.m_eFireTeamAssigned));
		
		return strGroupMemberCount + strEnemiesCount + strEnemiesToFireteams;
		return strGroupMemberCount + strEnemiesCount;
	}
};