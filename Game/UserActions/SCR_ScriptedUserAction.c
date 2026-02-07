//! A scripted action class having optional logic to check if vehicle is valid
class SCR_ScriptedUserAction : ScriptedUserAction
{
	[Attribute(desc: "Offset of action. Should be the same position as the Parent context (plus pivot point). Action offset is relative to the actionManager Owner and can be used for a variety of reasons such as checking nearby entities or playing audio.", uiwidget: UIWidgets.Coords, params: "inf inf 0 purpose=coords space=entity")]
	protected vector m_vActionOffset;
	
	[Attribute("0", desc: "When can the action be shown regarding the vehicle you are in. Note only checked for vehicle that the action is attached to. IGNORE will never check vehicle state", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EUserActionInVehicleState))]
	protected EUserActionInVehicleState m_eShownInVehicleState;
	
	[Attribute("1", desc: "Whether this action can only be performed by occupants of the same vehicle (if in vehicle)", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bSameVehicleOnly;
	
	//================================================== CAN SHOW ==================================================\\
	override bool CanBeShownScript(IEntity user)
	{
		//~ Check if in vehicle state is valid
		if (m_eShownInVehicleState != EUserActionInVehicleState.IGNORE)
		{	
			//~ Only check when character
			ChimeraCharacter character = ChimeraCharacter.Cast(user);
			if (character)
			{
				//~ Only check when has compartment access
				CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
				if (compAccess)
				{
					//~ Never show when getting in or out vehicle
					if (compAccess.IsGettingIn() || compAccess.IsGettingOut())
						return false;
					
					//~ Check if correct state
					switch (m_eShownInVehicleState)
					{
						//~ Character should not be in vehicle
						case EUserActionInVehicleState.NOT_IN_VEHICLE :
						{
							if (character.IsInVehicle())
								return false;
							
							break;
						}
						//~ Check if character is in current vehicle and is in any position
						case EUserActionInVehicleState.IN_VEHICLE_ANY :
						{
							if (!character.IsInVehicle())
								return false;
							
							break;
						}
						//~ Check if character is in current vehicle and pilot
						case EUserActionInVehicleState.IN_VEHICLE_PILOT :
						{
							if (!character.IsInVehicle() || SCR_CompartmentAccessComponent.GetCompartmentType(compAccess.GetCompartment()) != ECompartmentType.Pilot)
								return false;
							
							break;
						}
						//~ Check if character is in current vehicle and cargo
						case EUserActionInVehicleState.IN_VEHICLE_CARGO :
						{
							if (!character.IsInVehicle() || SCR_CompartmentAccessComponent.GetCompartmentType(compAccess.GetCompartment()) != ECompartmentType.Cargo)
								return false;
							
							break;
						}
						//~ Check if character is in current vehicle and turret
						case EUserActionInVehicleState.IN_VEHICLE_TURRET :
						{
							if (!character.IsInVehicle() || SCR_CompartmentAccessComponent.GetCompartmentType(compAccess.GetCompartment()) != ECompartmentType.Turret)
								return false;
							
							break;
						}
					}
				}
			}
		}
		
		// Disallow actions from being performed from the outside of a vehicle or the other way around, unless
		// they are in the same vehicle (if set)
		if (m_bSameVehicleOnly)
		{
			return IsSameVehicleOrNone(user);
		}

		//~ All passed so return true
		return true;
	}
	
	protected bool IsSameVehicleOrNone(notnull IEntity user)
	{
		
		// See if user can even access compartments
		CompartmentAccessComponent userCompAccess;
		
		// Use cached getter for character, but don't hinder logic for non-character users
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (userCharacter)
			userCompAccess = userCharacter.GetCompartmentAccessComponent();
		else
			 userCompAccess = CompartmentAccessComponent.Cast(user.FindComponent(CompartmentAccessComponent));
		
		
		if (!userCompAccess)
			return true; // Not really relevant
		
		IEntity owner = GetOwner();
		CompartmentAccessComponent ownerAccess = CompartmentAccessComponent.Cast(owner.FindComponent(CompartmentAccessComponent));
		if (!ownerAccess)
		{
			// If owner does not have comparment access, perhaps it is a vehicle itself?
			Vehicle vehicle = Vehicle.Cast(owner);
			if (!vehicle)
				return true; // No access, not relevant either
			
			IEntity userVehicle = userCompAccess.GetVehicleIn(user);
			return userVehicle == vehicle; // Same vehicle
		}
		
		// If both can access compartments, see if they are within the same vehicle
		IEntity userVehicle = userCompAccess.GetVehicleIn(user);
		IEntity ownerVehicle = ownerAccess.GetVehicleIn(owner);
		return userVehicle == ownerVehicle;
	}
	
	//================================================== ACTION POSITION ==================================================\\
	/*!
	Get action local position
	Used for such things like playing audio at the correct location on action use
	\return action local position
	*/
	vector GetActionLocalPositon()
	{
		return m_vActionOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get action world position
	Used for such things like playing audio at the correct location on action use
	\return action world position
	*/
	vector GetActionWorldPosition()
	{
		return GetOwner().CoordToParent(m_vActionOffset);
	}
	
	//================================================== INIT ==================================================\\
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (m_eShownInVehicleState != EUserActionInVehicleState.IGNORE)
		{
			if (!Vehicle.Cast(pOwnerEntity) && !Vehicle.Cast(pOwnerEntity.GetParent()))
				Print(string.Format("'SCR_ScriptedUserAction' m_eShownInVehicleState is '%1' but action is not attached to a vehicle (Or child of vehicle)!", typename.EnumToString(EUserActionInVehicleState, m_eShownInVehicleState)), LogLevel.ERROR);
		}
	}
};

//------------------------------------------------------------------------------------------------
//~ When can the action be shown
enum EUserActionInVehicleState
{
	IGNORE = 0,
	NOT_IN_VEHICLE = 10,
	IN_VEHICLE_ANY = 20,
	IN_VEHICLE_PILOT = 30,
	IN_VEHICLE_TURRET = 40,
	IN_VEHICLE_CARGO = 50,
};

