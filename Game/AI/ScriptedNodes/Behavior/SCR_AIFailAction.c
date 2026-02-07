class SCR_AIFailAction : SCR_AIActionTask
{

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!IsActionValid())
			return ENodeResult.FAIL;
		
		m_Action.Fail();
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
		return "Completes action specified in input or current action";
	}		

};