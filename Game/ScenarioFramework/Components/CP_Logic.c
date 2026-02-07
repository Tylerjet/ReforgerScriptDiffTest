
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class CP_LogicInput
{
	[Attribute( defvalue: "1", desc: "Input connector", UIWidgets.Auto, category: "Input" )];
	protected ref CP_ActionInputBase m_pInputAction;
	
	[Attribute( defvalue: "0", desc: "Input connector", UIWidgets.Auto, category: "Input" )];
	protected bool 			m_bLatch;
	
	protected bool			m_bSignal;
	protected IEntity		m_pEnt;
	protected CP_Logic		m_pMasterLogic;
	
		
	//------------------------------------------------------------------------------------------------
	void Init( CP_Logic pLogic )
	{
		m_pMasterLogic = pLogic;
		if ( m_pInputAction )
			m_pInputAction.Init( this );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate( bool bSignal )
	{
		if ( m_pMasterLogic )
			m_pMasterLogic.OnInput( bSignal );
	}
}


//------------------------------------------------------------------------------------------------
class CP_LogicClass: GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class CP_Logic: GenericEntity
{
	
	[Attribute( defvalue: "1", desc: "What causes the increase", UIWidgets.Auto, category: "Input" )];
	protected ref array<ref CP_LogicInput> m_aInputs;
		
	[Attribute( defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate" )];
	protected ref array<ref CP_ActionBase>	m_aActions;
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		foreach ( CP_LogicInput pInput : m_aInputs )
			pInput.Init( this );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInput( bool pSignal = true );
	
	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		foreach ( CP_ActionBase pAction : m_aActions )
			pAction.OnActivate( null );
	}
}



//------------------------------------------------------------------------------------------------
class CP_LogicCounterClass : CP_LogicClass
{
	// prefab properties here
}


class CP_LogicCounter : CP_Logic
{	
	[Attribute( defvalue: "1", desc: "Threshold", UIWidgets.Graph, category: "Counter" )];
	protected int							m_iCountTo;
	
	[Attribute( defvalue: "1", desc: "What to do once value is increased", UIWidgets.Auto, category: "OnIncrease" )];
	protected ref array<ref CP_ActionBase>	m_aOnIncreaseActions;
			
	protected int 							m_iCnt = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput( bool pSignal = true )
	{
		Increase();
	}
	
	//------------------------------------------------------------------------------------------------
	void Increase()
	{
		m_iCnt++;
		if ( m_iCnt == m_iCountTo )
		{
			OnActivate();
		}
		else
		{
			foreach ( CP_ActionBase pIncreaseAction : m_aOnIncreaseActions )
				pIncreaseAction.OnActivate( null );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_iCnt = 0;
	}
	
}


//------------------------------------------------------------------------------------------------
class CP_LogicORClass : CP_LogicClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class CP_LogicOR : CP_Logic
{
	protected int			m_iActivations = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput( bool pSignal = true )
	{
		if ( pSignal )
			m_iActivations = Math.Min( ++m_iActivations, m_aInputs.Count() );
		else
			m_iActivations--;
	}	
}


//------------------------------------------------------------------------------------------------
class CP_LogicSwitchClass : CP_LogicClass
{
	// prefab properties here
}


class CP_LogicSwitch : CP_Logic
{
		
}





