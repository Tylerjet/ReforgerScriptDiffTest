//------------------------------------------------------------------------------------------------
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
};

//------------------------------------------------------------------------------------------------
class VehicleClass: BaseVehicleClass
{
};

//------------------------------------------------------------------------------------------------
class Vehicle : BaseVehicle
{
	// Can be removed when EOnPhysicsActive is on components as well
	protected ref ScriptInvoker<IEntity, bool> m_OnPhysicsActive;
	
	[Attribute("0", UIWidgets.ComboBox, "Vehicle type:", "", ParamEnumArray.FromEnum(EVehicleType) )]
	EVehicleType m_eVehicleType;
	
	SCR_VehicleFactionAffiliationComponent m_pFactionComponent;
	
	//------------------------------------------------------------------------------------------------
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
	Faction GetFaction()
	{
		if (!m_pFactionComponent)
			m_pFactionComponent = SCR_VehicleFactionAffiliationComponent.Cast(FindComponent(SCR_VehicleFactionAffiliationComponent));
		
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsOccupied()
	{
		if (!m_pFactionComponent)
			m_pFactionComponent = SCR_VehicleFactionAffiliationComponent.Cast(FindComponent(SCR_VehicleFactionAffiliationComponent));
		
		if (m_pFactionComponent)
			return m_pFactionComponent.IsVehicleOccupied();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Vehicle(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.PHYSICSACTIVE);
	}
};