class SCR_WearClothUserAction : LoadoutUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		auto loadout = BaseLoadoutManagerComponent.Cast(character.FindComponent(BaseLoadoutManagerComponent));
		if (loadout)
			loadout.Wear(pOwnerEntity);
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		BaseLoadoutClothComponent cloth = GetClothComponent();
		if (!cloth)
			return false;
		
		BaseLoadoutManagerComponent loadout = BaseLoadoutManagerComponent.Cast(character.FindComponent(BaseLoadoutManagerComponent));
		if (!loadout)
			return false;
		
		if (!loadout.IsAreaAvailable(cloth.GetArea()))
			return false;
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		auto character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		if (character.FindComponent(SCR_InventoryStorageManagerComponent))
			return false;
		return true;
	}	
};