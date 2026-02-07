class SCR_AILookAtPosition: AITaskScripted
{

	[Attribute("5", UIWidgets.EditBox, "Random position from the center" )]
	float m_RandomRadius;

	[Attribute("20", UIWidgets.EditBox, "Priority of the look" )]
	float m_fPriority;

	static const float MINIMUM_AIM_DISTANCE_SQ = 2; // This will work only for infantry, not for tanks
	SCR_AIBehaviorBase m_Behavior;
	SCR_AIUtilityComponent m_Utility;
	vector m_vPosition;
	
	//-----------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//-----------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"Position",
		"RandomRadius",
		"Priority"
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		"LookPosition"
	};
	override array<string> GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//-----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Behavior)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (!m_Utility)
				return ENodeResult.FAIL;
			m_Behavior = SCR_AIBehaviorBase.Cast(m_Utility.m_CurrentBehavior);
			if (!m_Behavior)
				return ENodeResult.FAIL;
		}

		if (!GetVariableIn("Position",m_vPosition) || m_vPosition == vector.Zero)
			return ENodeResult.FAIL;

		if (!m_Utility.m_LookAction)
			return ENodeResult.FAIL;

		if (vector.DistanceSq(m_vPosition, m_Utility.GetOrigin()) < MINIMUM_AIM_DISTANCE_SQ)
		{
			// distance is too close to look at 
			return ENodeResult.SUCCESS;
		}	
		
		vector newPosition = s_AIRandomGenerator.GenerateRandomPointInRadius(0, m_RandomRadius, m_vPosition, true);
		newPosition[1] = m_vPosition[1] + 1; // Don't look at ground
		
		m_vPosition = newPosition;
		m_Utility.m_LookAction.LookAt(m_vPosition, m_fPriority);
		SetVariableOut("LookPosition",m_vPosition);
		return ENodeResult.SUCCESS;
    }
	
	//-----------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Look at position in radius of given position";
	}
};

