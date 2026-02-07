class SCR_AIFindAvailableVehicle: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_VEHICLE_OUT			= "VehicleOut";	
	static const string PORT_ROLE_OUT				= "RoleOut";	
	static const string PORT_SEARCH_PARAMS	    	= "SearchParams";		
		
	[Attribute("0", UIWidgets.CheckBox, "If found, add to list of usable vehicles?")]
	bool m_bAddToList;
	
	private BaseWorld m_world;
	private IEntity m_VehicleEntity;
	private SCR_AIBoardingWaypointParameters m_WaypointParameter;
	private ECompartmentType m_CompartmentType;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_world = owner.GetWorld();		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_world)
			return ENodeResult.FAIL;
		
		if (m_VehicleEntity && !HasNoAvailableCompartment(m_VehicleEntity)) // the vehicle was found already and it has available compartment
		{
			SetVariablesOut(owner, m_VehicleEntity, m_CompartmentType);
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
				SetVariablesOut(owner, m_VehicleEntity, m_CompartmentType);
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
		
		array<BaseCompartmentSlot> compartmentSlots = {};
		compComp.GetCompartments(compartmentSlots);
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			if (m_WaypointParameter.m_bIsDriverAllowed && PilotCompartmentSlot.Cast(slot))
				m_CompartmentType = ECompartmentType.Pilot;
			else if (m_WaypointParameter.m_bIsGunnerAllowed && TurretCompartmentSlot.Cast(slot))
				m_CompartmentType = ECompartmentType.Turret;
			else if (m_WaypointParameter.m_bIsCargoAllowed && CargoCompartmentSlot.Cast(slot))
				m_CompartmentType = ECompartmentType.Cargo;
			else 
				continue;
			
			if (!slot.AttachedOccupant() && slot.IsCompartmentAccessible())
			{
				slot.SetCompartmentAccessible(false);				
				m_VehicleEntity = ent;
				return false;
			}			
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
	void SetVariablesOut(AIAgent owner, IEntity vehicleOut, ECompartmentType compartmentTypeOut)
	{
		SetVariableOut(PORT_VEHICLE_OUT, m_VehicleEntity);
		SetVariableOut(PORT_ROLE_OUT, m_CompartmentType);
		if (m_bAddToList)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(owner);
			if (group)
			group.AddUsableVehicle(m_VehicleEntity);
		}
	}	
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE_OUT,
		PORT_ROLE_OUT
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
};