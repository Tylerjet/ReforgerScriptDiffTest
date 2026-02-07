class SCR_AIRemoveVehicleFromGetInActivity : AITaskScripted
{	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}

		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		if(!utility)
			return ENodeResult.FAIL;
		SCR_AIGetInActivity getIn =  SCR_AIGetInActivity.Cast(utility.GetCurrentAction());
		if (!getIn)
			return ENodeResult.FAIL;
		getIn.ClearActivityVehicle();
		
		// if there are no other vehicles, fail straight away without restarting the activity
		if (group.GetUsableVehiclesCount() == 0)
			getIn.Fail();
		
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
		return "Removes vehicle from get in activity";
	}
};