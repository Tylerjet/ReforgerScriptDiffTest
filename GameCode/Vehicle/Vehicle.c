enum EVehicleType
{
	VEHICLE 		= 0,
	CAR 			= 2,
	TRUCK 			= 4,
	APC 			= 8,
	FUEL_TRUCK 		= 16,
	COMM_TRUCK 		= 32,
	SUPPLY_TRUCK 	= 64,
	MORTAR			= 128,
	REPAIR			= 256,
	ARSENAL			= 512,
}

class VehicleClass : BaseVehicleClass
{
}

class Vehicle : BaseVehicle
{
	// Can be removed when EOnPhysicsActive is on components as well
	protected ref ScriptInvoker<IEntity, bool> m_OnPhysicsActive;
	
	[Attribute("0", UIWidgets.ComboBox, "Vehicle type:", "", ParamEnumArray.FromEnum(EVehicleType) )]
	EVehicleType m_eVehicleType;
	
	SCR_VehicleFactionAffiliationComponent m_pFactionComponent;
	protected BaseControllerComponent m_VehicleController;
	protected SCR_ResourceComponent m_ResourceComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnPhysicsActive()
	{
		if (!m_OnPhysicsActive)
			m_OnPhysicsActive = new ScriptInvoker();
		
		return m_OnPhysicsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (m_OnPhysicsActive)
			m_OnPhysicsActive.Invoke(owner, activeState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();
		
		return null;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_VehicleFactionAffiliationComponent GetFactionAffiliation()
	{
		return m_pFactionComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetDefaultFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetDefaultAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsOccupied()
	{	
		if (m_pFactionComponent)
			return m_pFactionComponent.IsVehicleOccupied();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get vehicle controller, stored as BaseControllerComponent
	BaseControllerComponent GetVehicleController()
	{
		return m_VehicleController;
	}

	//------------------------------------------------------------------------------------------------
	//! Get primary pilot compartment slot
	PilotCompartmentSlot GetPilotCompartmentSlot()
	{
		if (GetGame().GetIsClientAuthority())
		{
			VehicleControllerComponent controllerCA = VehicleControllerComponent.Cast(m_VehicleController);
			if (controllerCA)
				return controllerCA.GetPilotCompartmentSlot();
		}
		else
		{
			VehicleControllerComponent_SA controller = VehicleControllerComponent_SA.Cast(m_VehicleController);
			if (controller)
				return controller.GetPilotCompartmentSlot();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return pilot of the vehicle - occupant of the primary pilot compartment
	IEntity GetPilot()
	{
		PilotCompartmentSlot slot = GetPilotCompartmentSlot();
		if (slot)
			return slot.GetOccupant();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ResourceComponent GetResourceComponent()
	{
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateResourceComponent()
	{
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void Vehicle(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.PHYSICSACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (GetGame().GetIsClientAuthority())
			m_VehicleController = BaseControllerComponent.Cast(FindComponent(VehicleControllerComponent));
		else
			m_VehicleController = BaseControllerComponent.Cast(FindComponent(VehicleControllerComponent_SA));

		m_pFactionComponent = SCR_VehicleFactionAffiliationComponent.Cast(FindComponent(SCR_VehicleFactionAffiliationComponent));
		UpdateResourceComponent();
	}
}
