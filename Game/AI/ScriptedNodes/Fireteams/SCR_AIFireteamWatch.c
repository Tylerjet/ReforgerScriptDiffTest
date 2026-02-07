class SCR_AIFireteamWatch: AITaskScripted
{	
	static const string PORT_FIRETEAM		= "Fireteam";
	static const string PORT_ENEMEY		= "Enemy";
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	AIGroup m_thisGroup;

	int m_Fireteam;
		
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_thisGroup = AIGroup.Cast(owner);
		if (m_thisGroup)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(m_thisGroup.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_thisGroup)
			return ENodeResult.FAIL;
		
		if (!m_GroupUtilityComponent.IsFireteamsInitialized())
			return ENodeResult.RUNNING;
		
		GetVariableIn(PORT_FIRETEAM, m_Fireteam);
		
		if (m_Fireteam == 0)
			return ENodeResult.RUNNING;

		//if somebody alive keep going
		foreach (auto info : m_GroupUtilityComponent.m_aListOfAIInfo)
		{
			if (info.GetFireTeam() == m_Fireteam)
			{
				return ENodeResult.RUNNING;
			}	
		}
		IEntity targetEntity = null;
		GetVariableIn(PORT_ENEMEY, targetEntity);
		//return enemy and fail otherwise
		int targetIndex = m_GroupUtilityComponent.m_aListOfKnownEnemies.Find(targetEntity);
		if (targetIndex > -1)
			m_GroupUtilityComponent.m_aFireteamsForKnownEnemies[targetIndex] = 0;

		return ENodeResult.FAIL;
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Fails when there is no member of fireteam alive. Otherwise is running.";
	}		

	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_FIRETEAM,
		PORT_ENEMEY
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	override protected bool CanReturnRunning()
	{
		return true;
	}
};