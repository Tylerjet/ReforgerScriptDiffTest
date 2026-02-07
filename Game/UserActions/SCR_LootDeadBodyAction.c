class SCR_LootDeadBodyAction : SCR_LootAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return false;
		
		// Interacting with unconscious characters' inventory is forbidden due to AI, for now.
	/*	CharacterControllerComponent charControllerComp = char.GetCharacterController();
		if (!charControllerComp)
			return false;
		
		if (charControllerComp.IsUnconscious())
			return super.CanBePerformedScript(user);*/
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMan)
			return false;
		
		if (damageMan.GetState() != EDamageState.DESTROYED)
			return false;
		
		return super.CanBePerformedScript(user);
	}
};