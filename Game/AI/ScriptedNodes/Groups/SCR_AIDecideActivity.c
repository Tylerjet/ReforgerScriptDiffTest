class SCR_AIDecideActivity: AITaskScripted
{
	static const string PORT_GOAL_MESSAGE		= "GoalMessage";
	static const string PORT_INFO_MESSAGE		= "InfoMessage";
	static const string PORT_ACTIVITY_TREE		= "ActivityTree";
	static const string PORT_UPDATE_ACTIVITY	= "UpdateActivity";
	
	SCR_AIGroupUtilityComponent m_UtilityComponent;
	SCR_AIMessageGoal m_GoalMessage;
	SCR_AIMessageInfo m_InfoMessage;	
	SCR_AIActivityBase m_CurrentActivity;	
	private bool restartActivity;

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (owner)
			m_UtilityComponent = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComponent)
		{
			return NodeError(this, owner, "Group utility component is missing.");
		}

				
		GetVariableIn(PORT_GOAL_MESSAGE, m_GoalMessage);
		GetVariableIn(PORT_INFO_MESSAGE, m_InfoMessage);
				
						
		m_CurrentActivity = m_UtilityComponent.EvaluateActivity(m_GoalMessage, m_InfoMessage, restartActivity);
		if (!m_CurrentActivity || m_CurrentActivity.m_sBehaviorTree == ResourceName.Empty)
		{
			Print("AI: Missing behavior tree in " + m_CurrentActivity.ToString(), LogLevel.WARNING);
			return ENodeResult.FAIL;
		}
		
		if (restartActivity)
		{
			SetVariableOut(PORT_ACTIVITY_TREE, m_CurrentActivity.m_sBehaviorTree);
			SetVariableOut(PORT_UPDATE_ACTIVITY, restartActivity);
		}
		return ENodeResult.SUCCESS;	
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ACTIVITY_TREE,
		PORT_UPDATE_ACTIVITY
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_GOAL_MESSAGE,
		PORT_INFO_MESSAGE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
};