class SCR_AIIsValidAction : DecoratorScripted
{
	private SCR_AIBaseUtilityComponent m_Utility;
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
			if (!m_Utility)
				return false;
		}
		
		if (!m_Utility.m_ExecutedAction)
			return false;

		EAIActionState state = m_Utility.m_ExecutedAction.m_eState;
		return state != EAIActionState.COMPLETED && state != EAIActionState.FAILED;
	}

	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "Test if current action is still valid, or it was already completed or failed";
	}	
};