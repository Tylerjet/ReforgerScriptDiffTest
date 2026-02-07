class SCR_AIPerformSmartUserAction : AITaskScripted
{
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		"UserAction",
		"TargetEntity"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity targetEntity;
		string userActionString;
		typename userAction;
		GetVariableIn("TargetEntity", targetEntity);
		GetVariableIn("UserAction", userActionString);
		
		userAction = userActionString.ToType();
		if (!userAction)
			return ENodeResult.FAIL;	//NodeError(this, owner, "typename of Useraction null");
				
		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return ENodeResult.FAIL;
		
		if (!targetEntity)
			return ENodeResult.FAIL;
		
		ActionsManagerComponent actionOnEntity = ActionsManagerComponent.Cast(targetEntity.FindComponent(ActionsManagerComponent));
		array<BaseUserAction> outActions = new array<BaseUserAction>;
		actionOnEntity.GetActionsList(outActions);
		
		for (int index = 0, length = outActions.Count(); index < length; index ++)
		{
			ScriptedUserAction action = ScriptedUserAction.Cast(outActions[index]);
			if (action && userAction == action.Type() && action.CanBePerformedScript(controlledEntity))
			{
				action.PerformAction(targetEntity, controlledEntity);
				return ENodeResult.SUCCESS;
			}
		}
		return ENodeResult.FAIL;			
	}
};