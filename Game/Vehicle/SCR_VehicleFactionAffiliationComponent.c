class SCR_VehicleFactionAffiliationComponentClass: VehicleFactionAffiliationComponentClass
{
	
};

class SCR_VehicleFactionAffiliationComponent: VehicleFactionAffiliationComponent
{
	//! Local invokers for a specific vehicle
	private ref ScriptInvoker m_OnFactionUpdate;
	
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
	override protected void OnFactionChanged()
	{
		if (m_OnFactionUpdate)
			m_OnFactionUpdate.Invoke();
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
		m_iOccupantCount ++;
	}	
		
		
	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnCompartmentLeft(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move)
	{
		if (move) // moving only between compartments
			return;
		m_iOccupantCount --;
		if (!IsVehicleOccupied())
		{
			ClearAffiliatedFaction();
		}
	}
	
	// may return to DefaultAffiliatedFaction in some cases - override this
	//--------------------------------------------------------------------------------------------------------------------------
	void ClearAffiliatedFaction()
	{
		SetAffiliatedFaction(null); 
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFactionUpdate(bool createNew = true)
	{
		if (!m_OnFactionUpdate && createNew)
			m_OnFactionUpdate = new ScriptInvoker();
		return m_OnFactionUpdate;
	}
};