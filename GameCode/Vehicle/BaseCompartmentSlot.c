class BaseCompartmentSlot : ExtBaseCompartmentSlot
{
	private bool m_bCompartmentAccessible = true;

	bool IsCompartmentAccessible() 
	{
		return m_bCompartmentAccessible;
	}
	void SetCompartmentAccessible(bool val)
	{
		m_bCompartmentAccessible = val;
	}
	
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\return Vehicle entity
	*/
	IEntity GetVehicle()
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		while (owner)
		{
			if (owner.FindComponent(BaseCompartmentManagerComponent))
				vehicle = owner;
			
			owner = owner.GetParent();
		}
		return vehicle;
	}
	/*!
	Get vehicle this slot belongs to.
	May differ from GetOwner() if the slot is placed in vehicle's child entity, e.g., truck's cargo frame.
	\param[out] compartmentID Variable to be filled with ID of the compartment relative to the vehicle. May differ from GetCompartmentSlotID() which returns only ID within the entity the slot belong sto.
	\return Vehicle entity
	*/
	IEntity GetVehicle(out int compartmentID)
	{
		IEntity owner = GetOwner();
		IEntity vehicle;
		Managed component, componentCandidate;
		while (owner)
		{
			componentCandidate = owner.FindComponent(BaseCompartmentManagerComponent);
			if (componentCandidate)
			{
				vehicle = owner;
				component = componentCandidate;
			}
			
			owner = owner.GetParent();
		}
		if (vehicle)
		{
			BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(component);
			array<BaseCompartmentSlot> compartments = {};
			manager.GetCompartments(compartments);
			compartmentID = compartments.Find(this);
		}
		return vehicle;
	}
	
	void DamageOccupant(float damage, EDamageType damageType, IEntity instigator = null, bool gettingIn = false, bool gettingOut = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;
			
		RplComponent rpl = character.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;
		
		// Ignore characters that only began to get in the vehicle
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!gettingIn && access && access.IsGettingIn())
			return;
		
		if (!gettingOut && access && access.IsGettingOut())
			return;
		
		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (damageManager)
			damageManager.DamageRandomHitZones(damage, damageType, instigator);
	}
	
	void KillOccupant(IEntity instigator = null, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOccupant());
		if (!character)
			return;
		
		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!gettingIn && access && access.IsGettingIn())
			return;
		
		if (!gettingOut && access && access.IsGettingOut())
			return;
		
		if (eject && access)
		{
			access.EjectOutOfVehicle();
			controller.GetInputContext().SetVehicleCompartment(null);
			GetGame().GetCallqueue().CallLater(damageManager.Kill, 1, false, instigator);
		}
		else
		{
			damageManager.Kill(instigator)
		}
	}
};