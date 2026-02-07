class SCR_AIGetShotsFromEvents: AITaskScripted
{
	static const string PORT_NUMBER_OF_SHOTS = "NumberOfShots";
	
	SCR_AITargetStatisticsComponent m_Statistics;
	SCR_ChimeraAIAgent m_chimeraAgent;
	
	private int m_iNumberOfShots = 0;
	
	//----------------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!m_chimeraAgent)
			Debug.Error("Works only on CHimera ai agent!");
		
		IEntity ent = owner.GetControlledEntity();
		m_Statistics = SCR_AITargetStatisticsComponent.Cast(ent.FindComponent(SCR_AITargetStatisticsComponent));
		if (!m_Statistics)
			Debug.Error("Statistics component missing!");
	}
	
	//----------------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_iNumberOfShots = 0;	
	}
	
	//----------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Statistics || !m_chimeraAgent)
			return ENodeResult.FAIL;
/*		
		AIDangerEvent dangerEvent = m_chimeraAgent.GetLastDangerEvent();
		if (dangerEvent && dangerEvent.GetDangerType() ==  EAIDangerEventType.Danger_ProjectileHit && owner.GetControlledEntity() == dangerEvent.GetObject())
		{
			m_Statistics.AddShot(dangerEvent);
			m_iNumberOfShots += 1;
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_SHOT_STATISTICS))
				PrintFormat("Shot number %3 fired by %1 fell on %2 ", dangerEvent.GetObject(), dangerEvent.GetPosition(), m_iNumberOfShots);
			SetVariableOut(PORT_NUMBER_OF_SHOTS,m_iNumberOfShots)
		}	*/
		return ENodeResult.RUNNING;
	} 
	
	//----------------------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
	{
		return true;
	}	
	
	//----------------------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription() 
	{ 
		return "Get Shots from Events: collects percieved ProjectileHit danger events and stores them into SCR_AITargetStatisticsComponent, do NOT use with standard soldier tree!";
	};
	
	//----------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_NUMBER_OF_SHOTS
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//----------------------------------------------------------------------------------------------------------------
	override bool CanReturnRunning()
	{
		return true;
	}
};