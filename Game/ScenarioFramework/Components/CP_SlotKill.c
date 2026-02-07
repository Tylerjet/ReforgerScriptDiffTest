[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotKillClass : CP_SlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class CP_SlotKill : CP_SlotTask
{
	[Attribute( desc: "Waypoint Groups if applicable", category: "Waypoints" )]
 	protected ref array<ref SCR_WaypointSet> 	m_aWaypointGroupNames;	
	
	[Attribute( desc: "Spawn AI on the first WP Slot", defvalue: "1", category: "Waypoints" )]
 	protected bool								m_bSpawnAIOnWPPos;
	
	[Attribute( desc: "Default waypoint if any WP group is defined", "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", category: "Waypoints" )]
	protected ResourceName 						m_rWPToSpawn;
		
	protected ref array<AIWaypoint> 			m_aWaypoints = {};
	protected SCR_AIGroup						m_pAIGroup;
	
	//------------------------------------------------------------------------------------------------
	FactionKey GetFaction() { return m_sFaction; }
	
	//------------------------------------------------------------------------------------------------
	override void Init( CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true )
	{
		
		if ( EActivation == CP_EActivationType.SAME_AS_PARENT )
		{
			if ( m_aWaypointGroupNames.IsEmpty() && !m_rWPToSpawn )
				return;
			ScriptInvoker pInvoker;
			if ( pArea )
				pInvoker = pArea.GetOnAreaInit();
			
			if ( pInvoker )
				pInvoker.Insert( SetWPGroup );
		}
				
		if ( m_EActivationType != EActivation )
			return;
		
		SCR_AIGroup.IgnoreSpawning( true );

		super.Init( pArea, EActivation );
		
		StoreTaskSubjectToParentTaskLayer();
		
		if ( EActivation != CP_EActivationType.SAME_AS_PARENT )
			SetWPGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAIGroup()
	{
		EntitySpawnParams paramsPatrol = new EntitySpawnParams();
 		paramsPatrol.TransformMode = ETransformMode.WORLD;
		paramsPatrol.Transform[3] = m_pEntity.GetOrigin();
		m_pAIGroup = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab( Resource.Load( "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et" ) , null, paramsPatrol) );
		FactionAffiliationComponent facComp= FactionAffiliationComponent.Cast( m_pEntity.FindComponent( FactionAffiliationComponent) );
		if (!facComp)
			return;
		m_pAIGroup.SetFaction( facComp.GetAffiliatedFaction() );
		m_pAIGroup.AddAIEntityToGroup( m_pEntity, 0 );
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIWaypoint CreateDefaultWaypoint()
	{
		if ( !m_pEntity )
			return null;
		EntitySpawnParams paramsPatrolWP = new EntitySpawnParams();
		paramsPatrolWP.TransformMode = ETransformMode.WORLD;
		paramsPatrolWP.Transform[3] = m_pEntity.GetOrigin();
	
		Resource resWP = Resource.Load( m_rWPToSpawn );
		if ( !resWP )
			return null;
		AIWaypoint pWP = AIWaypoint.Cast( GetGame().SpawnEntityPrefab( resWP, null, paramsPatrolWP) );
		if ( !pWP )
			return null;
		m_aWaypoints.Insert( pWP );
		return pWP;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetWaypointsFromLayer( notnull CP_LayerBase pLayer, bool bRandomOrder )
	{
		//array<CP_LayerBase> aChildEnts = pLayer.GetChildrenEntities();
		array<IEntity> aEnts = pLayer.GetSpawnedEntities();
		if ( aEnts.IsEmpty() )
		{
			Print( "CP: no waypoints found!" );
			return;
		}
		m_aWaypoints.Resize( aEnts.Count() );
		int iIndex = -1;
		
		foreach( IEntity pEntity : aEnts )
		{
			if ( !AIWaypoint.Cast( pEntity ) )
				continue;
			if ( bRandomOrder )
			{
				Math.Randomize( - 1 );
				iIndex = Math.RandomInt( 0, aEnts.Count() );
				while ( m_aWaypoints[ iIndex ] )
				{
					iIndex = Math.RandomInt( 0, aEnts.Count() );
				}
			}
			else
			{			
				iIndex++;
			}
			
			m_aWaypoints.Set( iIndex, AIWaypoint.Cast( pEntity ) );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddCycleWaypoint()
	{
		EntitySpawnParams paramsPatrolWP = new EntitySpawnParams();
		paramsPatrolWP.TransformMode = ETransformMode.WORLD;
		paramsPatrolWP.Transform[3] = m_aWaypoints[0].GetOrigin();
	
		Resource resWP = Resource.Load( "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et" );
		if ( resWP )
			m_aWaypoints.Insert( AIWaypoint.Cast( GetGame().SpawnEntityPrefab( resWP, null, paramsPatrolWP) ) ); 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWPGroup()
	{
		if ( !m_pEntity )
		{
			Print( "CP: Trying to add waypoints to non existing entity! Did you select the object to spawn?" );
			SCR_AIGroup.IgnoreSpawning( false );
			return;
		}
		
		
		if ( !m_aWaypointGroupNames.IsEmpty() )
		{				
			//Select random layer which holds the waypoints ( defined in the layer setting )
			
			SCR_WaypointSet pWrap = m_aWaypointGroupNames.GetRandomElement();
			IEntity pEnt = GetGame().GetWorld().FindEntityByName( pWrap.m_sName );
			if ( pEnt )
			{
							
				CP_SlotBase pWP = CP_SlotBase.Cast( pEnt.FindComponent( CP_SlotBase ) );
				if ( pWP )
				{
					if ( AIWaypoint.Cast( pWP.GetSpawnedEntity() ) )
						m_aWaypoints.Insert( AIWaypoint.Cast( pWP.GetSpawnedEntity() ) );
				}
				else 			
				{
					CP_LayerBase pWPGroupLayer = CP_LayerBase.Cast( pEnt.FindComponent( CP_LayerBase ) );
					if ( pWPGroupLayer )
						GetWaypointsFromLayer( pWPGroupLayer, pWrap.m_bUseRandomOrder );
				}
				if ( !m_aWaypoints.IsEmpty() )
					if ( pWrap.m_bCycleWaypoints )
						AddCycleWaypoint();
			}
		}
		else
		{
			CreateDefaultWaypoint();
		}
				
		m_pAIGroup = SCR_AIGroup.Cast( m_pEntity );
		SCR_AIGroup.IgnoreSpawning( false );
		if ( !m_pAIGroup )
		{
			CreateAIGroup();
		}
		else
		{
			ActivateAI();
		}
		
		if ( m_aWaypoints.IsEmpty() )
			return;	
		AIWaypoint pWP;
		CP_SlotBase pObj;
		for ( int i = m_aWaypoints.Count() - 1 ; i >=0; i-- )
		{
			pWP = AIWaypoint.Cast( m_aWaypoints[ i ] );
			if ( !pWP )
			{
				Print( "CP: Problem happened while assigning a waypoint to group: waypoint not found!" );
				continue;
			}
			if ( AIWaypointCycle.Cast( pWP ) )
				AIWaypointCycle.Cast( pWP ).SetWaypoints( m_aWaypoints );
			m_pAIGroup.AddWaypoint( pWP );
		}
		if ( m_bSpawnAIOnWPPos )
			if ( !m_aWaypoints.IsEmpty() )
				m_pEntity.SetOrigin( m_aWaypoints[ m_aWaypoints.Count() - 1 ].GetOrigin() );
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateAI()
	{
		m_pAIGroup.SpawnUnits();		
	}

	//------------------------------------------------------------------------------------------------
	void CP_SlotKill(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~CP_SlotKill()
	{
	}
}
