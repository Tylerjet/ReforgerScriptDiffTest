class SCR_AIPerformSmartHealing : AITaskScripted
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
		typename userAction;
		IEntity targetEntity;
		string userActionString;
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
		
		// Early exit
		ChimeraCharacter char = ChimeraCharacter.Cast(targetEntity);
		if (char)
		{
			SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
			if (!damageMan && !damageMan.CanBeHealed())
				return ENodeResult.FAIL;
		}
		
		array<BaseUserAction> outActions = {};
		SCR_HealingUserAction action;
		GetActions(targetEntity, outActions);
		foreach (BaseUserAction baseAction : outActions)
		{
			// Check if action is healing action type
			action = SCR_HealingUserAction.Cast(baseAction);
			if (!action)
				continue;
			
			// Find the healing action on the correct hitzone group
			if (action.GetUserActionGroup() != GetTargetGroup(targetEntity))
				continue;
			
			// If healing action is of the desired healing-type, and can be performed, PerformAction
			if (action && userAction == action.Type() && action.CanBePerformedScript(controlledEntity))
			{
				action.PerformAction(targetEntity, controlledEntity);
				return ENodeResult.SUCCESS;
			}
		}
		return ENodeResult.FAIL;			
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetActions(IEntity targetEntity, notnull out array<BaseUserAction> outActions)
	{
		if (!targetEntity)
			return;
		
		ActionsManagerComponent actionOnEntity = ActionsManagerComponent.Cast(targetEntity.FindComponent(ActionsManagerComponent));
		
		if (!actionOnEntity)
			return;
		
		actionOnEntity.GetActionsList(outActions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected ECharacterHitZoneGroup GetTargetGroup(IEntity targetEntity)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(targetEntity);
		if (!char)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMan)
			return null;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMan.GetAllLimbs(limbs);
		SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(damageMan.GetMostDOTHitZone(EDamageType.BLEEDING, false, limbs));
		if (!charHitZone)
			return null;
		
		return charHitZone.GetHitZoneGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
};