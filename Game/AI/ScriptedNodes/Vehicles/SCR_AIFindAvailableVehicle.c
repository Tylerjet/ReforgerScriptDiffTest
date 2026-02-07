class SCR_AIFindAvailableVehicle: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_VEHICLE_OUT			= "VehicleOut";
	static const string PORT_ROLE_OUT				= "RoleOut";
	static const string PORT_COMPARTMENT_OUT		= "CompartmentOut";
	static const string PORT_SEARCH_PARAMS			= "SearchParams";
	
	[Attribute("0", UIWidgets.CheckBox, "If found, add to list of usable vehicles?")]
	bool m_bAddToList;
	
	[Attribute("1", UIWidgets.CheckBox, "If found, reserve compartment?")]
	bool m_bReserveCompartment;
	
	private BaseWorld m_world;
	private ref array<BaseCompartmentSlot> m_CompartmentSlots = {};
	protected IEntity m_VehicleEntity;
	protected BaseCompartmentSlot m_Compartment;
	protected ECompartmentType m_CompartmentType;
	protected SCR_AIBoardingWaypointParameters m_WaypointParameter;
	protected SCR_AIGroupUtilityComponent m_groupUtilityCompoment;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_world = owner.GetWorld();
		m_groupUtilityCompoment = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_world)
			return ENodeResult.FAIL;
		
		if (m_VehicleEntity && !HasNoAvailableCompartment(m_VehicleEntity)) // the vehicle was found already and it has available compartment
		{
			SetVariablesOut(owner, m_VehicleEntity, m_CompartmentType, m_Compartment);
			return ENodeResult.SUCCESS;
		}
		else
		{ 
			m_VehicleEntity = null;
			vector center;
			float radius;
			GetVariableIn(PORT_CENTER_OF_SEARCH, center);
			GetVariableIn(PORT_RADIUS, radius);
			GetVariableIn(PORT_SEARCH_PARAMS, m_WaypointParameter);
			
			m_world.QueryEntitiesBySphere(center, radius, HasNoAvailableCompartment, FilterEntities, EQueryEntitiesFlags.DYNAMIC);
			if (m_VehicleEntity)
			{
				if (m_bReserveCompartment)
				{
					SCR_AIGroup group = SCR_AIGroup.Cast(owner);
					if (!group)
					{
						return NodeError(this, owner, "GetEmptyCompartment not run on SCR_AIGroup agent!");
					}
					group.AllocateCompartment(m_Compartment);
				};
				SetVariablesOut(owner, m_VehicleEntity, m_CompartmentType, m_Compartment);
				return ENodeResult.SUCCESS;
			}
			ClearVariable(PORT_VEHICLE_OUT);
			ClearVariable(PORT_ROLE_OUT);
		};
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasNoAvailableCompartment(IEntity ent) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return true;
		
		compComp.GetCompartments(m_CompartmentSlots);
		if (m_CompartmentSlots.IsEmpty())
			return true;
		
		BaseCompartmentSlot pilotCompartment, turretCompartment, cargoCompartment;
		
		foreach (BaseCompartmentSlot slot : m_CompartmentSlots)
		{
			if (slot.GetOccupant() || !slot.IsCompartmentAccessible())
				continue;
			if (m_WaypointParameter.m_bIsDriverAllowed && PilotCompartmentSlot.Cast(slot))
				pilotCompartment = slot;
			else if (m_WaypointParameter.m_bIsGunnerAllowed && TurretCompartmentSlot.Cast(slot))
				turretCompartment = slot;
			else if (m_WaypointParameter.m_bIsCargoAllowed && CargoCompartmentSlot.Cast(slot))
				cargoCompartment = slot;
		}
		// going through priorities: pilot > turret > cargo
		if (pilotCompartment)
		{
			m_CompartmentType = ECompartmentType.Pilot;
			m_VehicleEntity = ent;
			m_Compartment = pilotCompartment;
			return false;
		}
		
		if (turretCompartment)
		{
			m_CompartmentType = ECompartmentType.Turret;
			m_VehicleEntity = ent;
			m_Compartment = turretCompartment;
			return false;
		}
		
		if (cargoCompartment)
		{
			m_CompartmentType = ECompartmentType.Cargo;
			m_VehicleEntity = ent;
			m_Compartment = cargoCompartment;
			return false;
		}
		return true; //continue search
	}
	
	//------------------------------------------------------------------------------------------------
	bool FilterEntities(IEntity ent) 
	{
		
		if (ent.FindComponent(BaseCompartmentManagerComponent))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVariablesOut(AIAgent owner, IEntity vehicleOut, ECompartmentType compartmentTypeOut, BaseCompartmentSlot compartment)
	{
		SetVariableOut(PORT_VEHICLE_OUT, vehicleOut);
		SetVariableOut(PORT_ROLE_OUT, compartmentTypeOut);
		SetVariableOut(PORT_COMPARTMENT_OUT, compartment);
		if (m_bAddToList)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(owner);
			if (group)
				group.AddUsableVehicle(vehicleOut);
		}
	}	
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE_OUT,
		PORT_ROLE_OUT,
		PORT_COMPARTMENT_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS,
		PORT_SEARCH_PARAMS
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "SCR_AIFindAvailableVehicle: finds within radius of origin available compartments of vehicles with priority pilot>turret>cargo";
	}
};