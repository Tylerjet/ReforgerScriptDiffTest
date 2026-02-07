[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "SCR_FuelConsumptionComponent", color: "0 0 255 255")]
class SCR_FuelConsumptionComponentClass: ScriptGameComponentClass
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Automatic fuel tank switching. Automatically select next fuel tank upon depletion of current fuel tank." )]
	bool m_bAutomaticFuelTankSwitching;
	
	[Attribute( defvalue: "20", uiwidget: UIWidgets.Auto, desc: "Fuel consumption at max power RPM [liters/hour]" )]
	float m_fFuelConsumption;
	
	[Attribute( defvalue: "0.5", uiwidget: UIWidgets.Auto, desc: "Fuel consumption idle [liters/hour]" )]
	float m_fFuelConsumptionIdle;
};

//------------------------------------------------------------------------------------------------
class SCR_FuelConsumptionComponent : ScriptGameComponent
{
	static const float							MIN_FUEL 			= 0;
	static const float							MIN_THRUST	 		= 0.04;
	static const float							MIN_CLUTCH		 	= 0.04;
	static const float							NEUTRAL_GEAR	 	= 1;
	static const float							TIME_STEP			= 1;
	
	private float								m_fTimeDelta;
	private BaseFuelNode						m_pCurrentFuelTank;
	private VehicleWheeledSimulation			m_pVehicleWheeledSim;
	private SCR_FuelConsumptionComponentClass	m_pComponentData;

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------

	
	//------------------------------------------------------------------------------------------------
	//! switch the actual fuel tank (i.e. by a switch on the dashboard)
	void SetCurrentFuelTank(int iFuelTankID)
	{
		m_pCurrentFuelTank = GetFuelTankByID(iFuelTankID);
	}
	
	//------------------------------------------------------------------------------------------------
	BaseFuelNode GetCurrentFuelTank()
	{
		return m_pCurrentFuelTank;
	}

	//------------------------------------------------------------------------------------------------
	//! returns the fuel tank pointer by the given FuelTankID
	private BaseFuelNode GetFuelTankByID(int iFuelTankID)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return null;
		
		array<BaseFuelNode> nodes = {};
		fuelManager.GetFuelNodesList(nodes);
		SCR_FuelNode scrNode;
		foreach (BaseFuelNode node: nodes)
		{
			// Currently ID is limited to SCR_FuelNode
			scrNode = SCR_FuelNode.Cast(node);
			if (scrNode && scrNode.GetFuelTankID() == iFuelTankID)
				return scrNode;
		}
		
		return null;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! returns the pointer of the first fuel tank with enough fuel (the fuel tank which is dedicated to the vehicle, not its cistern, cargo...)
	private BaseFuelNode FindNonEmptyFuelTank()
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return null;
		
		array<BaseFuelNode> nodes = {};
		fuelManager.GetFuelNodesList(nodes);
		foreach (BaseFuelNode node: nodes)
		{
			if (node && node.GetFuel() > MIN_FUEL)
				return node;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		if (!m_pComponentData)
			return;
		
		// Irrelevant on proxies
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl.IsProxy())
			return;
		
		if (enabled && GetEventMask() | EntityEvent.FIXEDFRAME)
			m_fTimeDelta = 0;
		
		if (m_pVehicleWheeledSim)
			Update();
		else
			enabled = false;
		
		if (enabled)
			SetEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
		else
			ClearEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! updates currently active fueltank to see if it can still be used. turns off engine if no usable fueltanks are present.
	void Update()
	{
		// if there is fuel in the actual tank
		if (!m_pCurrentFuelTank || m_pCurrentFuelTank.GetFuel() <= MIN_FUEL)
		{
			// auto switching not disabled
			if (m_pComponentData.m_bAutomaticFuelTankSwitching)
				m_pCurrentFuelTank = FindNonEmptyFuelTank();
			else
				m_pCurrentFuelTank = null;
		}
		
		// vehicle has at least one fuel tank
		if (m_pCurrentFuelTank)
		{
			float currentConsumption;
			float currentThrust = m_pVehicleWheeledSim.GetThrottle();
			if (currentThrust >= MIN_THRUST || (m_pVehicleWheeledSim.GetGear() != NEUTRAL_GEAR && m_pVehicleWheeledSim.GetClutch() >= MIN_CLUTCH))
				currentConsumption = m_pComponentData.m_fFuelConsumption * currentThrust * m_pVehicleWheeledSim.EngineGetRPM() / m_pVehicleWheeledSim.EngineGetRPMPeakPower();
			else
				currentConsumption = m_pComponentData.m_fFuelConsumptionIdle;
			
			// Scale hourly
			currentConsumption *= m_fTimeDelta * GAME_ACCELERATION_FACTOR / 3600;
			
			SCR_FuelNode scrFuelNode = SCR_FuelNode.Cast(m_pCurrentFuelTank);
			if (scrFuelNode)
				scrFuelNode.ConsumeFuelExt(currentConsumption);
			else
				m_pCurrentFuelTank.SetFuel(m_pCurrentFuelTank.GetFuel() - currentConsumption);
		}
		else
		{
			VehicleControllerComponent controller = VehicleControllerComponent.Cast(GetOwner().FindComponent(VehicleControllerComponent));
			if (controller)
				controller.StopEngine(false);
		}
		
		m_fTimeDelta = 0;
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		m_fTimeDelta += timeSlice;
		if (m_fTimeDelta > TIME_STEP)
			Update();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_pComponentData = SCR_FuelConsumptionComponentClass.Cast(GetComponentData(owner));
		if (!m_pComponentData)
			return;
		
		// Irrelevant on proxies
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl.IsProxy())
			SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return;
		
		m_pCurrentFuelTank = FindNonEmptyFuelTank();

		m_pVehicleWheeledSim = VehicleWheeledSimulation.Cast(owner.FindComponent(VehicleWheeledSimulation));
		if (m_pVehicleWheeledSim && !m_pVehicleWheeledSim.IsValid())
			m_pVehicleWheeledSim = null;
		
		if (m_pVehicleWheeledSim && m_pVehicleWheeledSim.EngineIsOn())
			SetEnabled(true);
	}
};
