class SCR_GetOutAction : SCR_CompartmentUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		BaseCompartmentSlot targetCompartment = GetCompartmentSlot();
		if (!targetCompartment)
			return;
		
		CompartmentAccessComponent compartmentAcess = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
		if (!compartmentAcess)
			return;
		
		if (!compartmentAcess.GetOutVehicle(GetRelevantDoorIndex(pUserEntity)))
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent));
		if (!characterController)
			return false;
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
			return false;
		
		if (!compartmentAccess.IsInCompartment())
			return false;
		
		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;
		
		BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
		if (compartment == null)
			return false;
		
		BaseCompartmentSlot thisCompartment = GetCompartmentSlot();
		return thisCompartment == compartment;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformed(user);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;
		
		UIInfo compartmentInfo = compartment.GetUIInfo();
		if (!compartmentInfo)
			return false;
		
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;
		
		outName = actionInfo.GetName();
		
		return true;
	}	
};