class SCR_VehicleFactionAffiliationComponentClass: SCR_FactionAffiliationComponentClass
{
	
};

class SCR_VehicleFactionAffiliationComponent: SCR_FactionAffiliationComponent
{
	private int m_iOccupantCount;
	
	//--------------------------------------------------------------------------------------------------------------------------
	override event void OnPostInit(IEntity owner)
	{
		ClearAffiliatedFaction();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	bool IsVehicleOccupied()
	{
		return m_iOccupantCount > 0;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnCompartmentEntered(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move)
	{
		if (move) // moving only between compartments
			return;
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(occupant);
		if(!character)
			return;
		CharacterControllerComponent controller = character.GetCharacterController();
		if(controller.IsDead())
			return;
		
		Faction characterFaction = character.GetFaction();
		if (characterFaction != null && !characterFaction.IsFactionFriendly(GetAffiliatedFaction())) // null faction is not friendly 
		{
			SetAffiliatedFaction(characterFaction);
		};
		m_iOccupantCount++;
	}	
		
		
	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnCompartmentLeft(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move)
	{
		if (move) // moving only between compartments
			return;
		m_iOccupantCount--;
		if (!IsVehicleOccupied())
		{
			ClearAffiliatedFaction();
		}
	}
};