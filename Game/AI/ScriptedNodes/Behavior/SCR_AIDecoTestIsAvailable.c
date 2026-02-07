class SCR_AIDecoTestIsAvailable : DecoratorTestScripted
{
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		if (controlled)
		{	
			AIControlComponent aiContr = AIControlComponent.Cast(controlled.FindComponent(AIControlComponent));
			if (!aiContr)
				return false;
			auto  infoComponent = SCR_AIInfoComponent.Cast(aiContr.GetAIAgent().FindComponent(SCR_AIInfoComponent));
			if (!infoComponent)
			{
				Debug.Error("Missing AIInfoComponent in for IEntity: " + controlled.ToString());
				return false;
			}
			return infoComponent.GetAIState() == EUnitAIState.AVAILABLE;
		}
		return false;	
	}
};