[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotTaskAIClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotTaskAI : SCR_ScenarioFrameworkSlotTask
{
	[Attribute(desc: "Waypoint Groups if applicable", category: "Waypoints")]
	ref SCR_ScenarioFrameworkWaypointSet m_WaypointSet;

	[Attribute(desc: "Spawn AI on the first WP Slot", defvalue: "1", category: "Waypoints")]
	bool m_bSpawnAIOnWPPos;

	[Attribute(desc: "Default waypoint if any WP group is defined", "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", category: "Waypoints")]
	ResourceName m_sWPToSpawn;

	[Attribute(defvalue: "0", desc: "Balancing group sizes based on number of players", category: "Balance")]
	bool m_bBalanceOnPlayersCount;

	[Attribute(defvalue: "1", desc: "Least amount of AIs in the group after balancing occurs. Will not exceed maximum number of units defined in the group prefab.", category: "Balance")]
	int m_iMinUnitsInGroup;

	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "Common")]
	SCR_EAIGroupFormation m_eAIGroupFormation;

	//[Attribute(defvalue: ECharacterStance.STAND.ToString(), UIWidgets.ComboBox, "AI character stance", "", ParamEnumArray.FromEnum(ECharacterStance), category: "Common")]
	//ECharacterStance m_eAICharacterStance;

	//[Attribute(defvalue: EMovementType.WALK.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(EMovementType), category: "Common")]
	//EMovementType m_eAIMovementType;

	[Attribute(defvalue: EAISkill.REGULAR.ToString(), UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill), category: "Common")]
	EAISkill m_eAISkill;

	[Attribute(defvalue: EAICombatType.NORMAL.ToString(), UIWidgets.ComboBox, "AI combat type", "", ParamEnumArray.FromEnum(EAICombatType), category: "Common")]
	EAICombatType m_eAICombatType;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.", params: "0 100 0.001", category: "Common")]
	float m_fPerceptionFactor;

	[Attribute(defvalue: "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", category: "Common")]
	ResourceName m_sGroupPrefab;

	ref array<AIWaypoint> m_aWaypoints = {};
	SCR_AIGroup m_AIGroup;
	ref array<ResourceName> m_aAIPrefabsForRemoval = {};

	ref array<SCR_ScenarioFrameworkSlotWaypoint> m_aSlotWaypoints = {};
	int m_iCurrentlySpawnedWaypoints;
	
	// Temporary solution as we don't know if invoker or a different method will initialize waypoints
	bool m_bWaypointsInitialized;

	//------------------------------------------------------------------------------------------------
	//! \param[in] arrayForRemoval
	void SetAIPrefabsForRemoval(array<ResourceName> arrayForRemoval)
	{
		m_aAIPrefabsForRemoval = arrayForRemoval;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ResourceName> GetAIPrefabsForRemoval()
	{
		return m_aAIPrefabsForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false)
	{
		m_aWaypoints.Clear();
		m_AIGroup = null;
		m_aAIPrefabsForRemoval.Clear();
		m_aSlotWaypoints.Clear();
		m_iCurrentlySpawnedWaypoints = 0;
		m_bWaypointsInitialized = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration);
	}

	//------------------------------------------------------------------------------------------------
	//!
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}

		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		if (m_Entity)
			m_vPosition = m_Entity.GetOrigin();

		if (m_AIGroup)
			m_AIGroup.GetOnAgentRemoved().Remove(DecreaseAIGroupMemberCount);

		m_AIGroup = null;

		m_aSpawnedEntities.RemoveItem(null);
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			m_vPosition = entity.GetOrigin();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
		m_aWaypoints.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		m_iCurrentlySpawnedWaypoints = 0;
		m_bWaypointsInitialized = false;
		SCR_AIGroup.IgnoreSpawning(true);

		if (m_eActivationType == SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT && m_WaypointSet && !m_WaypointSet.m_aLayerName.IsEmpty())
			InitWaypoints();
		
		super.Init(area, activation);
	}

	//------------------------------------------------------------------------------------------------
	//!
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		if (m_Entity)
			ActivateAI();
		else
			AfterAllAgentsSpawned();

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void AfterAllAgentsSpawned()
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);
		
		if (!m_bWaypointsInitialized)
			SetWaypointToAI(this);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ActivateAI()
	{
		bool groupWasNull;
		m_AIGroup = SCR_AIGroup.Cast(m_Entity);
		if (!m_AIGroup)
			groupWasNull = true;

		if (!m_AIGroup && !CreateAIGroup())
		{
			AfterAllAgentsSpawned();
			return;
		}

		if (!m_aAIPrefabsForRemoval.IsEmpty())
		{
			foreach (ResourceName prefabToRemove : m_aAIPrefabsForRemoval)
			{
				for (int i = m_AIGroup.m_aUnitPrefabSlots.Count() - 1; i >= 0; i--)
				{
					if (m_AIGroup.m_aUnitPrefabSlots[i] != prefabToRemove)
						continue;

					m_AIGroup.m_aUnitPrefabSlots.Remove(i);
					break;
				}
			}
		}
		else if (m_bBalanceOnPlayersCount)
		{
			int iMaxUnitsInGroup = m_AIGroup.m_aUnitPrefabSlots.Count();
			float iUnitsToSpawn = Math.Map(GetPlayersCount(), 1, GetMaxPlayersForGameMode(), Math.RandomInt(1, 3), iMaxUnitsInGroup);

			if (iUnitsToSpawn < m_iMinUnitsInGroup)
				iUnitsToSpawn = m_iMinUnitsInGroup;

			m_AIGroup.SetMaxUnitsToSpawn(iUnitsToSpawn);
		}

		m_AIGroup.GetOnAgentRemoved().Insert(DecreaseAIGroupMemberCount);

		if (groupWasNull)
		{
			m_AIGroup.SetNumberOfMembersToSpawn(1);
			OnAgentAdded(null);
			return;
		}

		m_AIGroup.SetMemberSpawnDelay(200);
		m_AIGroup.SpawnUnits();
		m_AIGroup.GetOnAgentAdded().Insert(OnAgentAdded);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnAgentAdded(AIAgent child)
	{
		if (m_AIGroup.GetNumberOfMembersToSpawn() != m_AIGroup.GetAgentsCount())
			return;

		m_AIGroup.GetOnAgentAdded().Remove(OnAgentAdded);

		InitGroupComponents();

		if (m_aSlotWaypoints.IsEmpty())
			SetWaypointToAI(this);

		AfterAllAgentsSpawned();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void InitGroupComponents()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);

		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (formComp)
		{
			formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, m_eAIGroupFormation));

			AIFormationDefinition formDef = formComp.GetFormation();
			if (formDef)
			{
				foreach (int i, AIAgent agent : agents)
				{
					IEntity agentEntity = agent.GetControlledEntity();
					if (!agentEntity)
						continue;

					if (m_vPosition != vector.Zero)
						agentEntity.SetOrigin(m_vPosition + formDef.GetOffsetPosition(i));
					else 
						agentEntity.SetOrigin(m_AIGroup.GetOrigin() + formDef.GetOffsetPosition(i));
					
					SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
					if (combatComponent)
					{
						combatComponent.SetAISkill(m_eAISkill);
						combatComponent.SetCombatType(m_eAICombatType);
						combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
					}
				}
			}
		}
		else
		{
			foreach (AIAgent agent : agents)
			{
				IEntity agentEntity = agent.GetControlledEntity();
				if (!agentEntity)
					continue;

				SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
				if (combatComponent)
				{
					combatComponent.SetAISkill(m_eAISkill);
					combatComponent.SetCombatType(m_eAICombatType);
					combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void InitWaypoints()
	{
		array<string> WaypointSetLayers = {};
		WaypointSetLayers = m_WaypointSet.m_aLayerName;
		array<SCR_ScenarioFrameworkLayerBase> layerBases = {};
		array<SCR_ScenarioFrameworkLayerBase> layerChildren = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		SCR_ScenarioFrameworkSlotWaypoint slotWaypoint;

		foreach (string waypointSetLayer : WaypointSetLayers)
		{
			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(waypointSetLayer);
			if (!layerEntity)
				continue;

			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase && !SCR_ScenarioFrameworkSlotWaypoint.Cast(layerBase))
			{
				if (layerBases.Contains(layerBase))
					continue;

				layerChildren = layerBase.GetChildrenEntities();
				foreach (SCR_ScenarioFrameworkLayerBase child : layerChildren)
				{
					slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(child);
					if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
				}
			}
			else
			{
				slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkSlotWaypoint));
				if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
			}
		}

		int waypointCount = m_aSlotWaypoints.Count();
		foreach (SCR_ScenarioFrameworkSlotWaypoint waypoint : m_aSlotWaypoints)
		{
			if (waypoint.GetIsInitiated())
				m_iCurrentlySpawnedWaypoints++;
			else
				waypoint.GetOnAllChildrenSpawned().Insert(CheckWaypointsAfterInit);

			if (waypointCount == m_iCurrentlySpawnedWaypoints)
				GetOnAllChildrenSpawned().Insert(ProcessWaypoints);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void CheckWaypointsAfterInit(SCR_ScenarioFrameworkLayerBase layer)
	{
		SCR_ScenarioFrameworkSlotWaypoint slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(layer);
		if (slotWaypoint)
		{
			SCR_ScenarioFrameworkWaypointCycle cycleWP = SCR_ScenarioFrameworkWaypointCycle.Cast(slotWaypoint.m_Waypoint);
			if (cycleWP)
			{
				if (!cycleWP.m_bInitiated)
				{
					cycleWP.GetOnAllWaypointsSpawned().Insert(CheckWaypointsAfterInit);
					return;
				}
				else if (cycleWP.m_OnAllWaypointsSpawned)
				{
					cycleWP.GetOnAllWaypointsSpawned().Remove(CheckWaypointsAfterInit);
				}
			}
		}

		m_iCurrentlySpawnedWaypoints++;
		if (m_iCurrentlySpawnedWaypoints == m_aSlotWaypoints.Count())
			ProcessWaypoints(this);

		layer.GetOnAllChildrenSpawned().Remove(CheckWaypointsAfterInit);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void ProcessWaypoints(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_bInitiated)
			SetWaypointToAI(this);
		else
			GetOnAllChildrenSpawned().Insert(SetWaypointToAI);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void SetWaypointToAI(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(SetWaypointToAI);
		m_bWaypointsInitialized = true;

		if (!m_Entity)
		{
			Print(string.Format("ScenarioFramework: Trying to add waypoints to non existing entity! Did you select the object to spawn for %1?", GetOwner().GetName()), LogLevel.ERROR);
			SCR_AIGroup.IgnoreSpawning(false);
			return;
		}

		if (m_aSlotWaypoints.IsEmpty())
		{
			CreateDefaultWaypoint();
		}
		else
		{
			AIWaypoint waypoint;
			foreach (SCR_ScenarioFrameworkSlotWaypoint waypointSlot : m_aSlotWaypoints)
			{
				waypoint = AIWaypoint.Cast(waypointSlot.GetSpawnedEntity());
				if (waypoint)
					m_aWaypoints.Insert(waypoint);
			}
		}

		if (!m_AIGroup)
			m_AIGroup = SCR_AIGroup.Cast(m_Entity);

		if (!m_AIGroup)
			return;
		
		m_aWaypoints.RemoveItemOrdered(null);

		foreach (AIWaypoint waypoint : m_aWaypoints)
		{
			m_AIGroup.AddWaypoint(waypoint);
		}

		if (m_vPosition != vector.Zero || !m_bSpawnAIOnWPPos || m_aWaypoints.IsEmpty())
			return;
		
		AIWaypoint waypoint = m_aWaypoints[0];
		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(waypoint);
		if (cycleWaypoint)
		{
			array<AIWaypoint> cycleWaypoints = {};
			cycleWaypoint.GetWaypoints(cycleWaypoints);
			if (!cycleWaypoints.IsEmpty() && cycleWaypoints[0] != null)
				waypoint = cycleWaypoints[0];
		}
			
		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (!formComp)
			return;
			
		AIFormationDefinition formDef = formComp.GetFormation();
		if (!formDef)
			return;
			
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);

		foreach (int i, AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
	
			agentEntity.SetOrigin(waypoint.GetOrigin() + formDef.GetOffsetPosition(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected AIWaypoint CreateDefaultWaypoint()
	{
		if (!m_Entity)
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = m_Entity.GetOrigin();

		Resource resWP = Resource.Load(m_sWPToSpawn);
		if (!resWP || !resWP.IsValid())
			return null;

		AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resWP, GetGame().GetWorld(), spawnParams));
		if (!waypoint)
			return null;

		m_aWaypoints.Insert(waypoint);
		return waypoint;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] group
	//! \param[in] agent
	void DecreaseAIGroupMemberCount(SCR_AIGroup group, AIAgent agent)
	{
		if (group.GetAgentsCount() == 0)
			SetIsTerminated(true);

		IEntity agentEntity = agent.GetControlledEntity();
		if (!agentEntity)
		{
			Print("ScenarioFramework: Decreasing AI member count failed due to not getting AI entity!", LogLevel.ERROR);
			return;
		}

		EntityPrefabData prefabData = agentEntity.GetPrefabData();
		if (!prefabData)
		{
			Print("ScenarioFramework: Decreasing AI member count failed due to not getting entity prefab data", LogLevel.ERROR);
			return;
		}

		ResourceName resource = prefabData.GetPrefabName();
		if (resource)
			m_aAIPrefabsForRemoval.Insert(resource);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected bool CreateAIGroup()
	{
		EntitySpawnParams paramsPatrol = new EntitySpawnParams();
		paramsPatrol.TransformMode = ETransformMode.WORLD;

		paramsPatrol.Transform[3] = m_Entity.GetOrigin();
		Resource groupResource = Resource.Load(m_sGroupPrefab);
		if (!groupResource.IsValid())
			return false;

		m_AIGroup = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(groupResource, GetGame().GetWorld(), paramsPatrol));
		if (!m_AIGroup)
			return false;

		FactionAffiliationComponent facComp = FactionAffiliationComponent.Cast(m_Entity.FindComponent(FactionAffiliationComponent));
		if (!facComp)
			return false;

		m_AIGroup.SetFaction(facComp.GetAffiliatedFaction());
		m_AIGroup.AddAIEntityToGroup(m_Entity);

		if (m_vPosition != vector.Zero)
			m_Entity.SetOrigin(m_vPosition);

		return true;
	}
}