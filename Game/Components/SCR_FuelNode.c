//---incoming liters per minute
enum EFuelFlowCapacityIn
{
	MANUAL = 			 10,
	VEHICLE_SMALL = 	 50,			//standard rate
	VEHICLE_MEDIUM = 	700,			//usually trucks rate
	VEHICLE_BIG = 		1400			//airplanes
};

//---outgoing liters per minute
enum EFuelFlowCapacityOut
{
	MANUAL = 			 10,
	VEHICLE_SMALL = 	 50,			//standard rate
	VEHICLE_MEDIUM = 	700,			//usually trucks rate
	VEHICLE_BIG = 		1400			//airplanes
};

class SCR_FuelNode : BaseFuelNode
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Initial fuel level" )]
	protected float m_fInitialFuelTankState;	//tank state of the entity after it's created

	[Attribute( defvalue: "50", uiwidget: UIWidgets.ComboBox, desc: "Maximum Flow Capacity\n[l/min]", enums: ParamEnumArray.FromEnum( EFuelFlowCapacityOut ) )]
	protected EFuelFlowCapacityOut m_MaxFlowCapacity;

	[Attribute( defvalue: "20", uiwidget: UIWidgets.CheckBox, desc: "Maximum Leak Speed\n[l/min]" )]
	protected int m_iFuelLeakSpeed;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Can refuel other vehicles?" )]
	protected bool m_bCanProvideFuel;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Can be refueled by other vehicles?" )]
	protected bool m_bCanReceiveFuel;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Fuel tank ID (user action and hitzone)" )]
	protected int m_iFuelTankID;	//for pairing with the user action

	static ref ScriptInvoker s_OnRefuelingFinished = new ref ScriptInvoker();

	#ifndef DISABLE_FUEL
	protected const float			TIME_STEP				= 1;
	protected float					m_fTimeSkip;											// Just for not overloading the onFrame each frame
	protected IEntity				m_Owner;												// Parent entity
	protected float					m_fHealth				= 1;							// Damage of fuel tank, reduces capacity

	protected SignalsManagerComponent m_SignalManagerComp;

	protected int					m_iSignalFuelStateIdx;
	protected int					m_iSignalIndexFuelTank;
	protected string				m_sSignalFuelState;
	protected const string			SIGNAL_FUEL_TANK_PREFIX				= "fuel";
	protected const string			SIGNAL_FUEL_TANK_ID					= "fueltank";

	#ifdef DEBUG_FUELSYSTEM
		private float 				m_fTestFuel = 0.0;
		private float 				m_fTestWorldTime = 0.0;
		[RplProp(condition: RplCondition.NoOwner, onRplName: "OnTestChanged")]
		private float				m_fDebugTankState 		= 0.0;
		private bool 				m_bFuelTankHUDAllowed	= true;
	#endif

	//------------------------------------------------------------------------------------------------
	//! True if fuel node can receive fuel
	bool CanReceiveFuel()
	{
		return m_bCanReceiveFuel;
	}

	//------------------------------------------------------------------------------------------------
	//! True if fuel node can provide fuel
	bool CanProvideFuel()
	{
		return m_bCanProvideFuel;
	}

	//------------------------------------------------------------------------------------------------
	//! Assigned fuel tank ID
	int GetFuelTankID()
	{
		return m_iFuelTankID;
	}

	//------------------------------------------------------------------------------------------------
	//! Owner entity of the fuel tank
	IEntity GetOwner()
	{
		return m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	void SetHealth(float health)
	{
		m_fHealth = Math.Clamp(health, 0, 1);

		if (m_fHealth == 1)
			return;

		if (ShouldSimulate())
			return;

		SetShouldSimulate(true);
		m_fTimeSkip = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFixedFrame(IEntity owner, float timeSlice)
	{
		// Work 1 time per second at most
		float leakableFuel;
		if (m_fHealth < 1)
			leakableFuel = GetLeakableFuel();

		if (leakableFuel > 0)
			m_fTimeSkip += timeSlice;
		else
			m_fTimeSkip = 0;

		if (m_fTimeSkip < TIME_STEP)
			return;

		// Reduce capacity of fuel tank by its damage
		// The fuel over reliable capacity will leak gradually
		float leak = Math.Min(leakableFuel, ((1 - m_fHealth) * m_iFuelLeakSpeed / 60) * m_fTimeSkip);

		SetFuel(GetFuel() - leak);

		m_fTimeSkip = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnFuelChanged(float newFuel)
	{
		if (m_SignalManagerComp)
			m_SignalManagerComp.SetSignalValue(m_iSignalFuelStateIdx, newFuel);
	}

	//------------------------------------------------------------------------------------------------
	float GetLeakableFuel()
	{
		return GetFuel() - GetMaxFuel() * m_fHealth;
	}

	//------------------------------------------------------------------------------------------------

	override void OnInit(IEntity owner)
	{
		m_Owner = owner;

		m_sSignalFuelState = SIGNAL_FUEL_TANK_PREFIX + GetFuelTankID().ToString();
		#ifdef DEBUG_FUELSYSTEM
			PrintFormat( "FUELSYSTEM: Inserting %1", this );
		#endif

		m_SignalManagerComp = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (m_SignalManagerComp)
		{
			m_iSignalFuelStateIdx = m_SignalManagerComp.AddOrFindSignal(m_sSignalFuelState);
			
			#ifdef DEBUG_FUELSYSTEM
				PrintFormat( "FUELSYSTEM: signal registered: %1 and set to: %2", m_sSignalFuelState, m_SignalManagerComp.GetSignalValue( m_iSignalFuelStateIdx ) );
			#endif
		}
		
		SetFuel(m_fInitialFuelTankState);
	}

	#else
	//Keeping just the declarations of function when the system is disabled
		bool CanReceiveFuel();
		bool CanProvideFuel();
		int GetFuelTankID();
		IEntity GetOwner();
		void SetHealth(float health);
		override void OnFixedFrame(IEntity owner, float timeSlice);
		protected void OnFuelChanged();
		float GetLeakableFuel();
		override void OnInit(IEntity owner);
	#endif
};
