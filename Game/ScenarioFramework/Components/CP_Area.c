[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_AreaClass : CP_LayerBaseClass
{
	
}


// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class CP_TaskType
{
	[Attribute( "Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum( ESFTaskType ) )];
	protected ESFTaskType m_eTypeOfTask;
	
	ESFTaskType GetTaskType() { return m_eTypeOfTask; }
}


/*
* Class is managing the area including the Slots ( prefab Slot.et ).
* The Slots should be placed in its hierarchy.
* GameModePatrolManager will select one instance of this type and will start to populate it based on the selected task
*
*/

class CP_Area : CP_LayerBase
{
	[Attribute( defvalue: "", UIWidgets.ResourcePickerThumbnail, "Trigger for area", category: "Trigger" )];
	protected ResourceName 					m_rTriggerResource;
	
	[Attribute( defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger area", category: "Trigger" )];
	protected float							m_fAreaRadius;
	
	protected SCR_BaseTriggerEntity											m_pTrigger;
	protected ref ScriptInvoker<CP_Area, CP_EActivationType>				m_pOnTriggerActivated;
	protected ref ScriptInvoker												m_pOnAreaInit = new ScriptInvoker();
	protected SCR_GameModeSFManager 										m_pGameModeManager;
	protected bool															m_bAreaSelected = false;
	protected SCR_BaseTask 													m_pTask;
	protected string	 													m_sDeliveryPointNameForItem;
	protected CP_LayerTask													m_pLayerTask;
	protected CP_SlotTask													m_pTaskSubject; 				//storing this one in order to get the task title and description
	
	//------------------------------------------------------------------------------------------------
	CP_SlotTask GetTaskSubject()
	{
		return m_pTaskSubject;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskSubject( CP_SlotTask pTaskSubject )
	{
		m_pTaskSubject = pTaskSubject;
	}
	//------------------------------------------------------------------------------------------------
	CP_LayerTask GetLayerTask()
	{
		return m_pLayerTask;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDeliveryPointName()
	{
		return m_sDeliveryPointNameForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreDeliveryPoint( string sDeliveryPointName )
	{
		m_sDeliveryPointNameForItem = sDeliveryPointName; 
		if ( !SCR_TaskDeliver.Cast( m_pTask ) )
			return;
		SCR_TaskDeliver.Cast( m_pTask ).SetTriggerNameToDeliver( sDeliveryPointName );
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreTaskToArea( SCR_BaseTask pTask )
	{
		m_pTask = pTask;
	}
	
	//------------------------------------------------------------------------------------------------
	CP_LayerTask Create()
	{
		array<CP_LayerBase> aSlotsOut = {};
		GetAllSlots( aSlotsOut );
		if ( aSlotsOut.IsEmpty() )
			return null;
		CP_LayerBase pLayer = CP_LayerBase.Cast( aSlotsOut.GetRandomElement() );
		if ( pLayer )
			pLayer.Init( this );
		return CP_LayerTask.Cast( pLayer );
	}
	
	//------------------------------------------------------------------------------------------------
	CP_LayerTask Create( ESFTaskType eTaskType )
	{
		array<CP_LayerBase> aSlotsOut = {};
		GetSuitableLayersForTaskType( aSlotsOut, eTaskType );
		if ( aSlotsOut.IsEmpty() )
			return null;
		//there might be more layers in the area conforming to the task type ( i.e. 2x the Truck task )
		m_pLayerTask = CP_LayerTask.Cast( aSlotsOut.GetRandomElement() );
		if ( m_pLayerTask )
			m_pLayerTask.Init( this, CP_EActivationType.ON_TASKS_INIT );
		return m_pLayerTask;
	}

	//------------------------------------------------------------------------------------------------
	void Create( CP_LayerTask pLayerTask )
	{
		//there might be more layers in the area conforming to the task type ( i.e. 2x the Truck task )
		m_pLayerTask = pLayerTask;
		if ( m_pLayerTask )
			m_pLayerTask.Init( this );
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnTrigger()
	{
		Resource resource = Resource.Load( m_rTriggerResource );
		if ( !resource )
			return;
		
		EntitySpawnParams pSpawnParams = new EntitySpawnParams();
		pSpawnParams.TransformMode = ETransformMode.WORLD;
		GetOwner().GetWorldTransform( pSpawnParams.Transform );
				
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles( pSpawnParams.Transform );
		Math3D.AnglesToMatrix(angles, pSpawnParams.Transform);
		
		
		//--- Spawn the prefab
		BaseResourceObject pResourceObject = resource.GetResource();
		if ( !pResourceObject )
			return;
		string resourceName = pResourceObject.GetResourceName();
		m_pTrigger = SCR_BaseTriggerEntity.Cast( GetGame().SpawnEntityPrefab( resource, GetGame().GetWorld(), pSpawnParams ) );
		if ( !m_pTrigger )
			return;
		
		//m_pTrigger.Show( false );
		m_pTrigger.SetSphereRadius( m_fAreaRadius );
		m_pTrigger.GetOnActivate().Insert( OnAreaTriggerActivated );
	}
	
	
	//------------------------------------------------------------------------------------------------
	void MoveTaskIconToArea( notnull SCR_BaseTask pTask )
	{
		pTask.SetOrigin( GetOwner().GetOrigin() );
		
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTaskSuitableForArea( ESFTaskType eTaskType )
	{
		array<ESFTaskType> aTaskTypes = {};
		GetAvailableTaskTypes( aTaskTypes );
		if ( aTaskTypes.IsEmpty() )
			return false;
		
		return aTaskTypes.Find( eTaskType ) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetAvailableTaskTypes( out array<ESFTaskType> aTaskTypes )
	{
		array<CP_LayerBase> aSlots = {};
		GetAllSlots( aSlots );
		ESFTaskType eType;
		CP_LayerTask pPos;
		foreach ( CP_LayerBase pLayer : aSlots )
		{
			pPos = CP_LayerTask.Cast( pLayer );
			if ( !pPos )
				continue;
			eType = pPos.GetTaskType();
			if ( aTaskTypes.Find( eType ) == -1 )
				aTaskTypes.Insert( eType );
		}	
	}
	
	
	//------------------------------------------------------------------------------------------------
	void GetSuitableLayersForTaskType( out notnull array<CP_LayerBase> aSlotsOut, ESFTaskType eTaskType )
	{
		ESFTaskType eType;
		array<CP_LayerBase> aSlots = {};
		GetAllSlots( aSlots );
		CP_LayerTask pPos;
		foreach ( CP_LayerBase pLayer : aSlots )
		{
			pPos = CP_LayerTask.Cast( pLayer );
			if ( !pPos )
				continue;
			eType = pPos.GetTaskType();
			if ( eTaskType == eType )
				aSlotsOut.Insert( pPos );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetAllSlots( out array<CP_LayerBase> aSlots )
	{
		CP_LayerBase pSlotComponent;
		IEntity child = GetOwner().GetChildren();
		while ( child )	
		{
			pSlotComponent = CP_LayerBase.Cast( child.FindComponent( CP_LayerBase ) );
			if ( pSlotComponent )
			{
				aSlots.Insert( pSlotComponent );
			}
			child = child.GetSibling();			
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SetAreaSelected( bool bSet ) { m_bAreaSelected = bSet; }
	
	//------------------------------------------------------------------------------------------------
	bool GetIsAreaSelected() { return m_bAreaSelected; }
		
	//------------------------------------------------------------------------------------------------
	void OnAreaTriggerActivated( IEntity pEnt )
	{
		if ( m_pOnTriggerActivated )
		{
			m_pOnTriggerActivated.Invoke( this, CP_EActivationType.ON_AREA_TRIGGER_ACTIVATION, false );
			m_pOnTriggerActivated.Clear();
			m_pTrigger.Deactivate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAreaTriggerActivated() 
	{
		if ( !m_pOnTriggerActivated )
			m_pOnTriggerActivated = new ScriptInvoker<CP_Area, CP_EActivationType>();
	
		return m_pOnTriggerActivated;	 
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAreaInit() {	return m_pOnAreaInit; }
	
	//------------------------------------------------------------------------------------------------
	override void Init( CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true )
	{
		if ( m_bInitiated )
			return;
		if ( m_EActivationType != CP_EActivationType.ON_INIT )
		{
			PrintFormat( "CP: Area %1 is set to %2 activation type, but area will always spawn on Init as default", GetOwner().GetName(), EActivation );
		}
		
		super.Init( this, CP_EActivationType.ON_INIT );		//area always spawned on the start;
		SpawnTrigger();	
		m_pOnAreaInit.Invoke();
	}

	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if ( m_pTrigger )
			super.EOnFrame( owner, timeSlice );
	}
			
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode pGameMode = SCR_BaseGameMode.Cast( GetGame().GetGameMode() );
		if ( !pGameMode )
			return;
	
		m_pGameModeManager = SCR_GameModeSFManager.Cast( pGameMode.FindComponent( SCR_GameModeSFManager ) );
		if ( m_pGameModeManager )
			m_pGameModeManager.RegisterArea( this );
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit( owner );
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if ( m_rTriggerResource )
			super._WB_AfterWorldUpdate( owner, timeSlice );
	}
#endif	

	//------------------------------------------------------------------------------------------------
	void CP_Area(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef WORKBENCH
			m_fDebugShapeColor = ARGB( 32, 0x99, 0xF3, 0x12 );;
			m_fDebugShapeRadius = m_fAreaRadius;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~CP_Area()
	{
	}
}
