[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotDefendClass : SCR_ScenarioFrameworkSlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_ScenarioFrameworkSlotDefend : SCR_ScenarioFrameworkSlotTask
{
	[Attribute(desc: "Waypoint Groups if applicable", category: "Waypoints")]
 	protected ref array<ref SCR_WaypointSet> 	m_aWaypointGroupNames;	
	
	[Attribute(desc: "Spawn AI on the first WP Slot", defvalue: "1", category: "Waypoints")]
 	protected bool								m_bSpawnAIOnWPPos;
	
	[Attribute(desc: "Default waypoint if any WP group is defined", "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", category: "Waypoints")]
	protected ResourceName 						m_sWPToSpawn;
	
	[Attribute(defvalue: "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", category:  "Waypoints")]
	protected ResourceName 										m_sGroupPrefab;	
	
	[Attribute(defvalue: "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et", category:  "Waypoints")]
	protected ResourceName 										m_sCycleWPPrefab;	
		
	protected ref array<AIWaypoint> 			m_aWaypoints = {};
	protected SCR_AIGroup						m_AIGroup;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		
		if (activation == SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
		{
			if (m_aWaypointGroupNames.IsEmpty() && !m_sWPToSpawn)
				return;
			
			ScriptInvoker invoker;
			if (area)
				invoker = area.GetOnAreaInit();
			
			if (invoker)
				invoker.Insert(SetWPGroup);
		}
				
		if (m_eActivationType != activation)
			return;
		
		SCR_AIGroup.IgnoreSpawning(true);

		super.Init(area, activation);
		
		if (activation != SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
			SetWPGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAIGroup()
	{
		EntitySpawnParams paramsPatrol = new EntitySpawnParams();
 		paramsPatrol.TransformMode = ETransformMode.WORLD;
		paramsPatrol.Transform[3] = m_Entity.GetOrigin();
		m_AIGroup = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_sGroupPrefab) , null, paramsPatrol));
		FactionAffiliationComponent facComp = FactionAffiliationComponent.Cast(m_Entity.FindComponent(FactionAffiliationComponent));
		if (!facComp)
			return;
		
		m_AIGroup.SetFaction(facComp.GetAffiliatedFaction());
		m_AIGroup.AddAIEntityToGroup(m_Entity, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIWaypoint CreateDefaultWaypoint()
	{
		if (!m_Entity)
			return null;
		
		EntitySpawnParams paramsPatrolWP = new EntitySpawnParams();
		paramsPatrolWP.TransformMode = ETransformMode.WORLD;
		paramsPatrolWP.Transform[3] = m_Entity.GetOrigin();
	
		Resource resWP = Resource.Load(m_sWPToSpawn);
		if (!resWP)
			return null;
		
		AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resWP, null, paramsPatrolWP));
		if (!waypoint)
			return null;
		
		m_aWaypoints.Insert(waypoint);
		return waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetWaypointsFromLayer(notnull SCR_ScenarioFrameworkLayerBase layer, bool randomOrder)
	{
		//array<SCR_ScenarioFrameworkLayerBase> aChildEnts = layer.GetChildrenEntities();
		array<IEntity> spawnedEntities = layer.GetSpawnedEntities();
		if (spawnedEntities.IsEmpty())
		{
			Print("ScenarioFramework: no waypoints found!", LogLevel.WARNING);
			return;
		}
		m_aWaypoints.Resize(spawnedEntities.Count());
		int index = -1;
		
		foreach (IEntity entity : spawnedEntities)
		{
			if (!AIWaypoint.Cast(entity))
				continue;
			
			if (randomOrder)
			{
				Math.Randomize(- 1);
				index = Math.RandomInt(0, spawnedEntities.Count());
				while (m_aWaypoints[index])
				{
					index = Math.RandomInt(0, spawnedEntities.Count());
				}
			}
			else
			{			
				index++;
			}
			
			m_aWaypoints.Set(index, AIWaypoint.Cast(entity));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddCycleWaypoint()
	{
		EntitySpawnParams paramsPatrolWP = new EntitySpawnParams();
		paramsPatrolWP.TransformMode = ETransformMode.WORLD;
		paramsPatrolWP.Transform[3] = m_aWaypoints[0].GetOrigin();
	
		Resource resWP = Resource.Load(m_sCycleWPPrefab);
		if (resWP)
			m_aWaypoints.Insert(AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resWP, null, paramsPatrolWP))); 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWPGroup()
	{
		if (!m_Entity)
		{
			Print("ScenarioFramework: Trying to add waypoints to non existing entity! Did you select the object to spawn?", LogLevel.WARNING);
			SCR_AIGroup.IgnoreSpawning(false);
			return;
		}
		
		if (!m_aWaypointGroupNames.IsEmpty())
		{				
			//Select random layer which holds the waypoints (defined in the layer setting)
			
			SCR_WaypointSet wrapper = m_aWaypointGroupNames.GetRandomElement();
			IEntity entity = GetGame().GetWorld().FindEntityByName(wrapper.m_sName);
			if (entity)
			{
				SCR_ScenarioFrameworkSlotBase waypoint = SCR_ScenarioFrameworkSlotBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotBase));
				if (waypoint)
				{
					if (AIWaypoint.Cast(waypoint.GetSpawnedEntity()))
						m_aWaypoints.Insert(AIWaypoint.Cast(waypoint.GetSpawnedEntity()));
				}
				else 			
				{
					SCR_ScenarioFrameworkLayerBase WPGroupLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
					if (WPGroupLayer)
						GetWaypointsFromLayer(WPGroupLayer, wrapper.m_bUseRandomOrder);
				}
				
				if (wrapper.m_bCycleWaypoints && !m_aWaypoints.IsEmpty())
					AddCycleWaypoint();
			}
		}
		else
		{
			CreateDefaultWaypoint();
		}
				
		m_AIGroup = SCR_AIGroup.Cast(m_Entity);
		SCR_AIGroup.IgnoreSpawning(false);
		if (!m_AIGroup)
			CreateAIGroup();
		else
			ActivateAI();
		
		if (m_aWaypoints.IsEmpty())
			return;
		
		AIWaypoint waypoint;
		SCR_ScenarioFrameworkSlotBase object;
		for (int i = m_aWaypoints.Count() - 1 ; i >=0; i--)
		{
			waypoint = AIWaypoint.Cast(m_aWaypoints[i]);
			if (!waypoint)
			{
				Print("ScenarioFramework: Problem happened while assigning a waypoint to group: waypoint not found!", LogLevel.ERROR);
				continue;
			}
			if (AIWaypointCycle.Cast(waypoint))
				AIWaypointCycle.Cast(waypoint).SetWaypoints(m_aWaypoints);
			
			m_AIGroup.AddWaypoint(waypoint);
		}
		if (m_bSpawnAIOnWPPos && !m_aWaypoints.IsEmpty())
			m_Entity.SetOrigin(m_aWaypoints[m_aWaypoints.Count() - 1].GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateAI()
	{
		m_AIGroup.SpawnUnits();		
	}
}
