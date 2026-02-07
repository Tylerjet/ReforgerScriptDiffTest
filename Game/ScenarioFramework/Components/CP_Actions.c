
//------------------------------------------------------------------------------------------------
class SCR_ContainerActionTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle( BaseContainer source, out string title )
	{
		title = source.GetClassName();
		title.Replace( "CP_Action", "" );
		string sOriginal = title;
		SplitStringByUpperCase( sOriginal, title );
		return true;
	}
	
	protected void SplitStringByUpperCase( string sInput, out string sOutput )
	{
		sOutput = "";
		int m;
		for ( int i = 0; i < sInput.Length(); i++ )
		{
			m = sInput.ToAscii( i );
			if ( m < 97 )	// lower case 
				sOutput += " ";
			sOutput += m.AsciiToString();
		}
	}
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//TODO: make this a generic action which can be used anywhere anytime ( i.e. on task finished, etc )
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionBase
{	
	protected IEntity				m_pEntity;
	
	//------------------------------------------------------------------------------------------------
	void Init( IEntity pEnt )
	{
		if ( !SCR_BaseTriggerEntity.Cast( pEnt ) )
			return;
		
		m_pEntity = pEnt;
		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast( pEnt ).GetOnActivate();
		if ( pOnActivateInvoker )
			pOnActivateInvoker.Insert( OnActivate );
		
		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast( pEnt ).GetOnDeactivate();
		if ( pOnDeactivateInvoker )
			pOnDeactivateInvoker.Insert( OnActivate );		//registering OnDeactivate to OnActivate - we need both changes 
	}	
	
	//------------------------------------------------------------------------------------------------
	void OnActivate( IEntity pObject );
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnObjects( notnull array<string> aObjectsNames, CP_EActivationType eActivationType )
	{ 
		IEntity pObj;
		CP_LayerBase pLayer;
		
		foreach ( string sObjectName : aObjectsNames )
		{
			pObj = GetGame().GetWorld().FindEntityByName( sObjectName );
			if ( !pObj )
			{
				PrintFormat( "CP: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName );
				continue;
			}
			pLayer = CP_LayerBase.Cast( pObj.FindComponent( CP_LayerBase ) );
			if ( !pLayer )
			{
				PrintFormat( "CP: Can't spawn object - the slot doesn't have CP_LayerBase component", sObjectName );
				continue;
			}
			pLayer.Init( null, eActivationType, false );
		}
	}
}	

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionIncrementCounter : CP_ActionBase
{
	[Attribute( defvalue: "", UIWidgets.EditBox, desc: "Counter to increment", category: "")];
	protected string				m_sCounterName;
	
	//------------------------------------------------------------------------------------------------
	override void Init( IEntity pEnt )
	{
		if ( !m_sCounterName )
			return;
		super.Init( pEnt );		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		IEntity pEnt = GetGame().GetWorld().FindEntityByName( m_sCounterName );
		if ( !pEnt )
			return;
		CP_Logic pCounter = CP_Logic.Cast( pEnt.FindComponent( CP_Logic ) );
		if ( !pCounter )
		{
			string className = pEnt.ClassName();
			if (className == "CP_LogicCounter")
			{
				CP_LogicCounter logicCounter = CP_LogicCounter.Cast(pEnt);
				logicCounter.OnInput(1);
				return;
			}
			else
			{
				return;
			}
		}
		pCounter.OnInput( 1 );
	}	
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionSpawnObjects : CP_ActionBase
{
	[Attribute( defvalue: "", UIWidgets.EditComboBox, desc: "These objects will spawn once the trigger becomes active.", category: "")];
	protected ref array<string> 	m_sNameOfObjectsToSpawnOnActivation;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		SpawnObjects( m_sNameOfObjectsToSpawnOnActivation, CP_EActivationType.ON_TRIGGER_ACTIVATION );
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionEndMission : CP_ActionBase
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pManager )
			return;
		pManager.Finish();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionCompareCounterAndExecute : CP_ActionBase
{

	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(CP_EOperatorCompare))]
	protected CP_EOperatorCompare			m_EOperatorCompare;
	
	[Attribute(desc: "Value")];
	protected int							m_iValue;

	[Attribute( defvalue: "", UIWidgets.EditBox, desc: "Counter to increment", category: "")];
	protected string						m_sCounterName;
	
	[Attribute(defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate")];
	protected ref array<ref CP_ActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity pObject)
	{
		int counterValue;
		
		IEntity pEnt = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!pEnt)
			return;
		
		string className = pEnt.ClassName();
		if (className == "CP_LogicCounter")
		{
			CP_LogicCounter logicCounter = CP_LogicCounter.Cast(pEnt);
			counterValue = logicCounter.m_iCnt;
		}	
		
		if ( 
				(( m_EOperatorCompare == CP_EOperatorCompare.LESS_THAN) 			&& (counterValue < m_iValue )) 	||
				(( m_EOperatorCompare == CP_EOperatorCompare.LESS_OR_EQUAL) 		&& (counterValue <= m_iValue )) ||
				(( m_EOperatorCompare == CP_EOperatorCompare.EQUAL) 				&& (counterValue == m_iValue )) ||
				(( m_EOperatorCompare == CP_EOperatorCompare.GREATER_OR_EQUAL) 		&& (counterValue >= m_iValue )) ||
				(( m_EOperatorCompare == CP_EOperatorCompare.GREATER_THEN) 			&& (counterValue > m_iValue )) 
			)
		{
			foreach (CP_ActionBase actions : m_aActions)
				actions.OnActivate(null);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionSetMissionEndScreen : CP_ActionBase
{
	
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))];
	protected EGameOverTypes			m_eGameOverType;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity pObject)
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!pManager)
			return;
		pManager.SetMissionEndScreen(m_eGameOverType);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionShowHint : CP_ActionBase
{
	[Attribute()];
	protected string		m_sTitle;
	
	[Attribute()];
	protected string		m_sText;
	
		
	[Attribute( defvalue: "15" )];
	protected int			m_iTimeout;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		SCR_HintManagerComponent.ShowCustomHint( m_sText, m_sTitle, m_iTimeout );
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionSpawnClosestObjectFromList : CP_ActionBase
{
	[Attribute( desc: "Closest to what - use getter" )];
	protected ref CP_Get		m_pGetter;
	
	[Attribute( defvalue: "", UIWidgets.EditComboBox, desc: "The closest one from the list will be spawned", category: "")];
	protected ref array<string> 	m_sListOfObjects;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		float fDistance = float.MAX;
		if ( !m_pGetter )
		{
			Print( "CP: The object the distance is calculated from is missing!" );
			return;
		}
		CP_Param<IEntity> pEntWrapper =  CP_Param<IEntity>.Cast( m_pGetter.Get() );
		if ( !pEntWrapper )
			return;
		IEntity pEntityFrom = IEntity.Cast( pEntWrapper.GetValue() );
		IEntity pClosestEntity; 
		IEntity pEntityInList;
		CP_LayerBase pSelectedLayer;
		if (!pEntityFrom)
		{
			Print("CP: Action: Getter returned null object. Random object spawned instead.");
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_sListOfObjects[m_sListOfObjects.GetRandomIndex()]);
			
			pEntityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!pEntityInList)
			{
				PrintFormat("CP: Object %1 doesn't exist", aRandomObjectToSpawn[0]);
				return;
			}
			
			SpawnObjects(aRandomObjectToSpawn, CP_EActivationType.ON_TRIGGER_ACTIVATION);
			return;
		}
		
		foreach ( string sObjectName : m_sListOfObjects )
		{
			pEntityInList = GetGame().GetWorld().FindEntityByName( sObjectName );
			if ( !pEntityInList )
			{
				PrintFormat( "CP: Object %1 doesn't exist", sObjectName );
				continue;
			}
			
			float fActualDistance = Math.AbsFloat( vector.Distance( pEntityFrom.GetOrigin(), pEntityInList.GetOrigin() ) );
			PrintFormat( "CP: actual distance: %1", fActualDistance );
		
			if (  fActualDistance < fDistance )
			{
				pClosestEntity = pEntityInList;
				fDistance = fActualDistance;				
			}
		}
		
		if ( !pClosestEntity )
			return;

		pSelectedLayer = CP_LayerBase.Cast( pClosestEntity.FindComponent( CP_LayerBase ) );
		
		if ( pSelectedLayer )
		{
			pSelectedLayer.Init( null, CP_EActivationType.ON_TRIGGER_ACTIVATION, false );
		}
		else
		{
			PrintFormat( "CP: Can't spawn slot %1 - the slot doesn't have CP_LayerBase component", pClosestEntity.GetName() );
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionPlaySound : CP_ActionBase
{
	[Attribute( defvalue: "", desc: "Sound to play.", category: "Action" )]		
	protected string 			m_sSound;
			
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pManager )
			return;
		
		GetGame().GetCallqueue().CallLater( pManager.PlaySoundOnEntity, 2000, false, null, m_sSound );		
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionPlaySoundOnEntity : CP_ActionBase
{
	[Attribute( desc: "Entity to play the sound on" )];
	protected ref CP_Get		m_pGetter;
	
	[Attribute( defvalue: "", desc: "Sound to play.", category: "Action" )]		
	protected string 			m_sSound;
			
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pManager )
			return;
		CP_Param<IEntity> pEntWrapper =  CP_Param<IEntity>.Cast( m_pGetter.Get() );
		if ( !pEntWrapper )
			return;
		GetGame().GetCallqueue().CallLater( pManager.PlaySoundOnEntity, 2000, false, IEntity.Cast( pEntWrapper.GetValue() ), m_sSound );		
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionResetCounter : CP_ActionBase
{	
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		if ( !pObject )
			return;
		CP_LogicCounter pCounter = CP_LogicCounter.Cast( pObject.FindComponent( CP_LogicCounter ) );
		if ( pCounter )
			pCounter.Reset();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionExecuteFunction : CP_ActionBase
{
	[Attribute( desc: "Object the method will be called" )];
	protected ref CP_Get		m_pObjectToCallTheMethodFrom;
	
	[Attribute( desc: "Method to call" )];
	protected string			m_sMethodToCall;
	
	[Attribute( desc: "Parameter to pass (string only)" )];
	protected string		m_sParameter;

			
	//------------------------------------------------------------------------------------------------
	override void OnActivate( IEntity pObject )
	{
		CP_Param<IEntity> pEntWrapper =  CP_Param<IEntity>.Cast( m_pObjectToCallTheMethodFrom.Get() );
		if ( !pEntWrapper )
			return;
		GetGame().GetCallqueue().CallByName( pEntWrapper.GetValue(), m_sMethodToCall, m_sParameter );
	}
}

