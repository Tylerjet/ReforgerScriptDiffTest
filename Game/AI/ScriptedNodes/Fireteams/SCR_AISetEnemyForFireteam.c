class SCR_AISetEnemyToFireteam: AITaskScripted
{
	static const string PORT_TARGET_IN				= "TargetIn";
	static const string PORT_POSITION_IN			= "PositionIn";
	static const string PORT_FIRETEAM_OUT			= "FireteamOut";
	static const string PORT_ACT_INDIVIDUALLY_OUT	= "ActIndividuallyOut";
	static const int NUMBER_OF_FIRE_TEAMS_PER_GROUP	= 4;
	
	
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
		
		int numberOfEnemies = m_GroupUtilityComponent.m_aListOfKnownEnemies.Count();
		int numberOfGroupMembers = m_GroupUtilityComponent.m_aListOfAIInfo.Count();
		int enemyCountPerFireTeam[NUMBER_OF_FIRE_TEAMS_PER_GROUP];
		int indexOfFireTeam;
		IEntity targetEntity;
		
		if (!GetVariableIn(PORT_TARGET_IN,targetEntity))
			return ENodeResult.FAIL;
		
		int targetIndex = m_GroupUtilityComponent.m_aListOfKnownEnemies.Find(targetEntity);
		if (targetIndex < 0)
			return ENodeResult.FAIL;
		vector lastSeenPos;
		GetVariableIn(PORT_POSITION_IN,lastSeenPos); // using perception info from contact message
		m_GroupUtilityComponent.m_aPositionsForKnownEnemies[targetIndex] = lastSeenPos;
		
		if (numberOfEnemies >= numberOfGroupMembers) 
		{
			ClearVariable(PORT_FIRETEAM_OUT);
			SetVariableOut(PORT_ACT_INDIVIDUALLY_OUT,true);
			return ENodeResult.SUCCESS;
		}
		// how many enemies are assigned in each fireteam?
		for (int i = 0; i< numberOfEnemies; i++)
		{
			enemyCountPerFireTeam[m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[i]] = enemyCountPerFireTeam[m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[i]] + 1;			
		}
		// unused fireteams dont have a count
		for (int i = m_GroupUtilityComponent.GetNumberOfFireTeams() + 1; i < NUMBER_OF_FIRE_TEAMS_PER_GROUP; i++)
		{	
			enemyCountPerFireTeam[i] = -1;
		}	
		// finding fire team with least number of assigned enemies
		
		for (int i = 0; i< numberOfEnemies; i++)
		{
			if (m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[i] == EFireTeams.NONE)
			{
				FindLeastUsedFireTeam(enemyCountPerFireTeam,indexOfFireTeam);
				m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[i]=indexOfFireTeam;
				enemyCountPerFireTeam[EFireTeams.NONE] = enemyCountPerFireTeam[EFireTeams.NONE] - 1;
				enemyCountPerFireTeam[indexOfFireTeam] = enemyCountPerFireTeam[indexOfFireTeam] + 1;
			}	
		}	 
		SetVariableOut(PORT_FIRETEAM_OUT,m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[targetIndex]);
		SetVariableOut(PORT_ACT_INDIVIDUALLY_OUT,false);		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	void FindLeastUsedFireTeam(int fireTeamsCount[NUMBER_OF_FIRE_TEAMS_PER_GROUP],out int fireTeamIndex)
	{
		int minValue = int.MAX;
		
		for (int i = 1; i< NUMBER_OF_FIRE_TEAMS_PER_GROUP; i++)
		{
			if (fireTeamsCount[i] < minValue && fireTeamsCount[i] > -1)
			{
				minValue = fireTeamsCount[i];
				fireTeamIndex = i;
			} 		
		}
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
		PORT_TARGET_IN,
		PORT_POSITION_IN
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
		
		string strEnemiesCount = string.Format("Enemies: %1\n", m_GroupUtilityComponent.m_aListOfKnownEnemies.Count());
		
		string strEnemiesToFireteams = "Enemies -> Fireteams:\n";
		foreach (EFireTeams f, int i : m_GroupUtilityComponent.m_aFireteamsForKnownEnemies)
			strEnemiesToFireteams = strEnemiesToFireteams + string.Format("  %1: %2 %3\n", i, f, typename.EnumToString(EFireTeams, f));
		
		return strGroupMemberCount + strEnemiesCount + strEnemiesToFireteams;
	}
};