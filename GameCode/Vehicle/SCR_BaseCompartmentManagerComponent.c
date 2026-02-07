class SCR_BaseCompartmentManagerComponentClass: BaseCompartmentManagerComponentClass
{
};

class SCR_BaseCompartmentManagerComponent : BaseCompartmentManagerComponent
{
	/*!
	Get compartments of specific type
	*/
	void GetCompartmentsOfType(inout array<BaseCompartmentSlot> outCompartments, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == compartmentType)
				outCompartments.Insert(compartment);
		}
	}
	
	/*!
	Get occupants of all managed compartments.
	*/
	void GetOccupants(inout array<IEntity> occupants)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get occupants of all managed compartments of specified type.
	void GetOccupantsOfType(inout array<IEntity> occupants, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartmentsOfType(compartments, compartmentType);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage random hitzones depending on amount of damage to be applied. Must be called on authority.
	void DamageOccupants(float damage, EDamageType damageType, IEntity instigator = null, bool gettingIn = false, bool gettingOut = false)
	{
		if (damage == 0)
			return;
		
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment)
				compartment.DamageOccupant(damage, damageType, instigator, gettingIn, gettingOut);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Kill all the occupants and eject them if requested. Must be called on authority.
	void KillOccupants(IEntity instigator = null, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment)
				compartment.KillOccupant(instigator, eject, gettingIn, gettingOut);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set AI compartment access for all compartments
	void SetCompartmentsAccessibleForAI(bool accessible)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
			compartment.SetCompartmentAccessible(accessible);
	}
};