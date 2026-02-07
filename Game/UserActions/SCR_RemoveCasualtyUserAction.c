class SCR_RemoveCasualtyUserAction : SCR_CompartmentUserAction
{
	const string m_sCannotPerformHostile = "#AR-UserAction_SeatHostile";
	const string m_sCannotPerformObstructed = "#AR-UserAction_SeatObstructed";

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

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;

		int index = GetRelevantDoorIndex(pUserEntity);

		CompartmentDoorInfo doorInfo = targetCompartment.GetDoorInfo(index);

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return;

		IEntity targetCasualty = compartment.GetOccupant();

		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(targetCasualty);
		if (!targetCharacter)
			return;

		CompartmentAccessComponent casualtyCompartmentAccess = targetCharacter.GetCompartmentAccessComponent();
		if (!casualtyCompartmentAccess)
			return;

		casualtyCompartmentAccess.EjectOutOfVehicle();

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		IEntity owner = compartment.GetOwner();
		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(owner, true));
		if (vehicle)
		{
			Faction characterFaction = character.GetFaction();
			Faction vehicleFaction = vehicle.GetFaction();
			
			if (characterFaction && vehicleFaction && characterFaction.IsFactionEnemy(vehicleFaction))
			{
				bool isActive = true;
				SCR_VehicleFactionAffiliationComponent vehicleAffiliation = vehicle.GetFactionAffiliation();
				if (vehicleAffiliation)
					isActive = vehicleAffiliation.IsVehicleActive();
				
				if (isActive)
				{
					SetCannotPerformReason(m_sCannotPerformHostile);
					return false;
				}
			}
		}

		// Make sure vehicle can be enter via provided door, if not, set reason.
		if (!character.IsInVehicle() && !compartmentAccess.CanGetInVehicleViaDoor(owner, compartment, GetRelevantDoorIndex(user)))
		{
			SetCannotPerformReason(m_sCannotPerformObstructed);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;

		IEntity occupant = compartment.GetOccupant();
		if (!occupant)
			return false;

		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(compartment.GetOccupant());
		if (!targetCharacter)
			return false;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(targetCharacter.GetCharacterController());
		if (!controller)
			return false;
		
		return controller.GetLifeState() != ECharacterLifeState.ALIVE;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
};
