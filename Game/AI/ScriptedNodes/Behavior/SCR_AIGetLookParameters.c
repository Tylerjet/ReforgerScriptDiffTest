class SCR_AIGetLookParameters : AITaskScripted
{
	static const string PORT_CAN_LOOK = "CanLook";
	static const string PORT_RESET_LOOK = "ResetLook";
	static const string PORT_LOOK_POSITION = "LookPosition";
	
	SCR_AILookAction m_LookAction;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_LookAction)
			return ENodeResult.FAIL;

		if (m_LookAction.m_bCanLook)
			SetVariableOut(PORT_CAN_LOOK, true);
		else
			ClearVariable(PORT_CAN_LOOK);
		
		if (m_LookAction.m_bResetLook)
		{
			SetVariableOut(PORT_RESET_LOOK, true);
			m_LookAction.m_bResetLook = false;
		}
		else
			ClearVariable(PORT_RESET_LOOK);

		if (m_LookAction.m_bValidLookPosition && m_LookAction.m_vPosition == vector.Zero)
			Print("AI: looking at 0 0 0", LogLevel.WARNING);
		
		if (!m_LookAction.m_bValidLookPosition)
			ClearVariable(PORT_LOOK_POSITION);
		else
			SetVariableOut(PORT_LOOK_POSITION, m_LookAction.m_vPosition);
		
		return ENodeResult.SUCCESS;
	}

	protected override bool VisibleInPalette() {return true;}
	protected override string GetOnHoverDescription() {return "Gets parameters of a lookAction";}
	
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utility)
			m_LookAction = utility.m_LookAction;
	}

	protected static ref TStringArray s_aVarsOut = {
		PORT_CAN_LOOK,
		PORT_RESET_LOOK,
		PORT_LOOK_POSITION
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};