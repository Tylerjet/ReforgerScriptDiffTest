class SCR_AIInitFireTeams: AITaskScripted
{
	static const string PORT_BLUE_TEAM		= "BlueTeam";
	static const string PORT_RED_TEAM		= "RedTeam";
	static const string PORT_GREEN_TEAM		= "GreenTeam";
	static const string PORT_YELLOW_TEAM	= "YellowTeam";
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	AIGroup m_thisGroup;

	// Right now this node could be reduced to just the GetNumberOfFireTeams
	// But it is dependent on FireTeams solution which isn't generalized right now
	// TODO generalize FireTeams
		
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
		
		m_GroupUtilityComponent.InitFireTeams();
		
		switch (m_GroupUtilityComponent.GetNumberOfFireTeams())
		{
			case 1:
			{
				SetVariableOut(PORT_BLUE_TEAM,1);
				ClearVariable(PORT_RED_TEAM);
				ClearVariable(PORT_GREEN_TEAM);
				ClearVariable(PORT_YELLOW_TEAM);
				break;				
			}
			case 2:
			{
				SetVariableOut(PORT_BLUE_TEAM,1);
				SetVariableOut(PORT_RED_TEAM,2);
				ClearVariable(PORT_GREEN_TEAM);
				ClearVariable(PORT_YELLOW_TEAM);
				break;				
			}
			case 3:
			{
				SetVariableOut(PORT_BLUE_TEAM,1);
				SetVariableOut(PORT_RED_TEAM,2);
				SetVariableOut(PORT_GREEN_TEAM,3);
				ClearVariable(PORT_YELLOW_TEAM);
				break;				
			}
			case 4:
			{
				SetVariableOut(PORT_BLUE_TEAM,1);
				SetVariableOut(PORT_RED_TEAM,2);
				SetVariableOut(PORT_GREEN_TEAM,3);
				SetVariableOut(PORT_YELLOW_TEAM,4);
				break;				
			}	
			default:
			{
				NodeError(this, owner, "More fire teams than supported right now.");
				break;
			}	
		};
		return ENodeResult.SUCCESS;
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_BLUE_TEAM,
		PORT_RED_TEAM,
		PORT_GREEN_TEAM,
		PORT_YELLOW_TEAM
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }	
};