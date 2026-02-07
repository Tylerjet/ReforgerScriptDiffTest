class SCR_AIPerformSmartUserAction : AITaskScripted
{
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"UserAction",
		"TargetEntity"
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity targetEntity;
		string userActionString;
		typename userAction;
		GetVariableIn("TargetEntity", targetEntity);
		GetVariableIn("UserAction", userActionString);

		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return ENodeResult.FAIL;

		if (!targetEntity)
			return ENodeResult.FAIL;

		userAction = userActionString.ToType();
		if (!userAction)
			return ENodeResult.FAIL;

		array<BaseUserAction> outActions = {};
		ScriptedUserAction action;
		GetActions(targetEntity, outActions);
		foreach (BaseUserAction baseAction : outActions)
		{
			action = ScriptedUserAction.Cast(baseAction);
			if (action && userAction == action.Type() && action.CanBePerformedScript(controlledEntity))
			{
				action.PerformAction(targetEntity, controlledEntity);
				return ENodeResult.SUCCESS;
			}
		}
		return ENodeResult.FAIL;			
	}
	
	//------------------------------------------------------------------------------------------------
	private void GetActions(IEntity targetEntity, notnull out array<BaseUserAction> outActions)
	{
		if (!targetEntity)
			return;
		
		ActionsManagerComponent actionOnEntity = ActionsManagerComponent.Cast(targetEntity.FindComponent(ActionsManagerComponent));
		
		if (!actionOnEntity)
			return;
		
		actionOnEntity.GetActionsList(outActions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
};