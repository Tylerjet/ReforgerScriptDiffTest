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

private const float 	GAME_ACCELERATION_FACTOR 		= 1.0;

class SCR_FuelNode: BaseFuelNode
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Fuel tank state (actual)" )]
	private float m_fInitialFuelTankState;	//tank state of the entity after it's created
	[Attribute( defvalue: "50", uiwidget: UIWidgets.ComboBox, desc: "Maximum Flow Capacity", enums: ParamEnumArray.FromEnum( EFuelFlowCapacityOut ) )]
	private EFuelFlowCapacityOut m_MaxFlowCapacity;
	[Attribute( defvalue: "300", uiwidget: UIWidgets.CheckBox, desc: "Liters per minute when leaking" )]
	private int m_iFuelLeakSpeed;
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Can refuel other vehicles?" )]
	private bool m_bCanProvideFuel;
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Can be refueled by other vehicles?" )]
	private bool m_bCanReceiveFuel;
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Tank ID (user action and hitzone)" )]
	private int m_iFuelTankID;	//for pairing with the user action
	[Attribute( defvalue: "0.9", uiwidget: UIWidgets.EditBox, desc: "The threshold at which the component is considered broken")]
	private float m_fMaxDamageScaled;


	static ref ScriptInvoker s_OnRefuelingFinished = new ref ScriptInvoker();
	
	#ifndef DISABLE_FUEL
	private const float			MAX_TIME_STEP			= 10; 							// can very well be just bool, these should be updated only when transferring or leaking the fuel.
	private const float			MIN_TIME_STEP			= 1;
	private float				m_fTimeSkip;											// Just for not overloading the onFrame each frame
	private float				m_fTimeStep = MAX_TIME_STEP;							// Allow lazy update for inactive entities
	private static ref array<SCR_FuelNode> m_aGlobalFuelNodes = new ref array<SCR_FuelNode>();
	private string 				m_sActionNameText;
	private IEntity				m_Owner;												// Parent entity
	private float				m_fHealth				= 1;							// Damage of fuel tank, reduces capacity
	
	private SignalsManagerComponent m_SignalManagerComp;
	//private SignalsManagerComponent m_SignalManagerCompProvider;
	private int 					m_iSignalFuelStateIdx;
	private int 					m_iSignalIndexFuelTank;
	private const string			m_sSignalFuelState_prefix			= "fuel";
	private string					m_sSignalFuelState;	//TODO: fix it, not used
	private const string			SIGNAL_FUEL_TANK_ID					= "fueltank";	
	
	private BaseHUDComponent 			m_HUDManagerComponentDashBoard;
	private SCR_HUDManagerComponent 	m_HUDManagerComponent;							//temporary for showing HUD of the fuel providers
	private ref array<BaseInfoDisplay> 	m_InfoDisplays 			= new ref array<BaseInfoDisplay>;
	private ref array<BaseInfoDisplay> 	m_InfoDisplaysDashboard	= new ref array<BaseInfoDisplay>;
	private SCR_AnalogGaugeUni 			m_FuelGauge;
	private SCR_AnalogGaugeMultiSignal	m_FuelGaugeDashboard;
	
	#ifdef DEBUG_FUELSYSTEM
		private float 				m_fTestFuel = 0.0;
		private float 				m_fTestWorldTime = 0.0;
		[RplProp(condition: RplCondition.NoOwner, onRplName: "OnTestChanged")]
		private float				m_fDebugTankState 		= 0.0;
		private bool 				m_bFuelTankHUDAllowed	= true;
	#endif
	
	//------------------------------------------------------------------------------------------------
	//!
	bool IsTypeOfReceiver()
	{
		return m_bCanReceiveFuel;
	}
	//------------------------------------------------------------------------------------------------
	//!
	bool IsTypeOfProvider()
	{
		return m_bCanProvideFuel;
	}

	//------------------------------------------------------------------------------------------------
	//!
	int GetFuelTankID()
	{
		return m_iFuelTankID;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	IEntity GetOwner()
	{
		return m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	void ConsumeFuel( float value )
	{
		SetFuel(GetFuel() - value);
	}
	
	//------------------------------------------------------------------------------------------------
	void ConsumeFuelExt( float value )
	{
		ConsumeFuel(value);
		/*
		if ( m_FuelGaugeDashboard )
			m_FuelGaugeDashboard.SwitchFuelTank( "fuel" + GetFuelTankID() );
		*/
		if ( m_SignalManagerComp )
		{
			m_SignalManagerComp.SetSignalValue( m_iSignalFuelStateIdx, GetFuel() );
			m_SignalManagerComp.SetSignalValue( m_iSignalIndexFuelTank, GetFuelTankID() );
			//PrintFormat( "FUELSYSTEM: fuelTank signal ID: %1", m_SignalManagerComp.GetSignalValue( m_iSignalIndexFuelTank ) );
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetHealth(float health)
	{
		m_fHealth = Math.Clamp(health, 0, 1);
		
		if (m_fHealth < 1)
			SetShouldSimulate(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFixedFrame( IEntity owner, float timeSlice )
	{
		// Work 1 time per second at most
		m_fTimeSkip += timeSlice;

		// TODO: add event mask
		if (m_fTimeSkip < m_fTimeStep)
			return;
		
		// Simulate leaks
		if (m_fHealth < 1)
		{
			// Reduce capacity of fuel tank by its damage
			float fLeakableFuel = GetFuel() - GetMaxFuel() * m_fHealth;
			if (fLeakableFuel > 0)
			{
				// The fuel over reliable capacity will leak gradually
				float fLeak = Math.Min(fLeakableFuel, ((1 - m_fHealth) * m_iFuelLeakSpeed / 60) * m_fTimeSkip * GAME_ACCELERATION_FACTOR);
				ConsumeFuelExt(fLeak);
				// Simulate as if fuel tank was actively being used
				m_fTimeStep = MIN_TIME_STEP;
			}
		}
		
		m_fTimeSkip = 0;
	}

	//------------------------------------------------------------------------------------------------
	
	void EOnDiag( IEntity owner, float timeslice )
	{
	}

	//------------------------------------------------------------------------------------------------
	
	override void OnInit(IEntity owner)
	{
		m_Owner = owner;
		
		m_sSignalFuelState = m_sSignalFuelState_prefix + ( this.GetFuelTankID()).ToString();
		#ifdef DEBUG_FUELSYSTEM
			PrintFormat( "FUELSYSTEM: Inserting %1", this );
		#endif
		if ( m_aGlobalFuelNodes )
			m_aGlobalFuelNodes.Insert( this );
		
		SetFuel( m_fInitialFuelTankState );
		
		m_SignalManagerComp = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
		//PrintFormat( "FUELSYSTEM: owner: %1, SignalManager: %2", owner.GetName(), m_SignalManagerComp );
		if ( !m_SignalManagerComp )
			return;
				
		m_iSignalFuelStateIdx = m_SignalManagerComp.AddOrFindSignal( m_sSignalFuelState);
		m_SignalManagerComp.SetSignalValue( m_iSignalFuelStateIdx, GetFuel() );
		m_iSignalIndexFuelTank = m_SignalManagerComp.AddOrFindSignal( SIGNAL_FUEL_TANK_ID);
		
		#ifdef DEBUG_FUELSYSTEM
			PrintFormat( "FUELSYSTEM: signal registered: %1 and set to: %2 | %3", m_sSignalFuelState, m_SignalManagerComp.GetSignalValue( m_iSignalFuelStateIdx ), GetFuel() );
		#endif
		//PrintFormat( "FUELSYSTEM: signal registered: %1 and set to: %2 | %3", m_sSignalFuelState_prefix + (this.GetFuelTankID()).ToString(), m_SignalManagerComp.GetSignalValue( m_iSignalFuelStateIdx ), GetFuel() );
		
		//---debug HUD
		if ( !this.IsTypeOfProvider() )
			return;
		
		m_HUDManagerComponent = SCR_HUDManagerComponent.Cast( owner.FindComponent( SCR_HUDManagerComponent ) );
		
		if ( !m_HUDManagerComponent )
			return;
		
		if ( m_HUDManagerComponent.GetInfoDisplays( m_InfoDisplays ) == 0 )
			return;
		
		m_FuelGauge = SCR_AnalogGaugeUni.Cast( m_InfoDisplays.Get(0) );
		
		m_HUDManagerComponentDashBoard = BaseHUDComponent.Cast( owner.FindComponent( BaseHUDComponent ) );
		if ( ! m_HUDManagerComponentDashBoard )
			return;
		if ( m_HUDManagerComponentDashBoard.GetInfoDisplays( m_InfoDisplaysDashboard ) == 0 ) 	
			return;
		
		//TODO: find it by the SLOT_FUEL property 
		m_FuelGaugeDashboard = SCR_AnalogGaugeMultiSignal.Cast( m_InfoDisplaysDashboard.Get( 0 ) );
		
		/*
		m_SignalManagerCompProvider = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		PrintFormat( "FUELSYSTEM: Entity: SignalsManagerComponent = %1", m_SignalManagerCompProvider );
		if ( !m_SignalManagerCompProvider )
			return;
		m_iSignalIndex = m_SignalManagerCompProvider.AddOrFindSignal( "fuel" );
		*/
	}
	
	//-----------------------------------------------------------------------------------------------------------------------
	//! JIP on server
	bool RplSave(ScriptBitWriter writer)
	{
		return true;
	}
	//-----------------------------------------------------------------------------------------------------------------------
	//! JIP on client
	bool RplLoad(ScriptBitReader reader)
	{	
		return true;
	}
	
	#else
	//Keeping just the declarations of function when the system is disabled
		void ListGlobalFuelNodeArray();
        string GetActionNameText();
        bool IsTypeOfReceiver();
        bool IsTypeOfProvider();
        int GetFuelTankID();
        IEntity GetOwner();
        void ConsumeFuelExt( float value );
        void SetHealth(float health);
        override void OnFixedFrame( IEntity owner, float timeSlice );
        void EOnDiag( IEntity owner, float timeslice );
        bool OnTicksOnRemoteProxy();
        override void OnInit(IEntity owner);
        override bool RplSave(ScriptBitWriter writer);
        override bool RplLoad(ScriptBitReader reader);
        void SCR_FuelNode( IEntityComponentSource src, IEntity ent, IEntity parent );
	#endif
};