class SCR_AISetBehaviorInterruptable : SCR_AIActionTask
{
	[Attribute("true", UIWidgets.CheckBox, "IsInterruptable")]
	private bool m_IsInterruptable;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!IsActionValid())
			return ENodeResult.FAIL;
		
		m_Action.SetActionInterruptable(m_IsInterruptable);
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Sets behavior to be /not be interruptable by another behavior";
	}		

	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_IsInterruptable)
			return "INTERRUPTABLE";
		else
			return "NOT INTERRUPTABLE";		
	}
};