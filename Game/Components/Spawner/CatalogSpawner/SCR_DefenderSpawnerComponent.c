[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component to be used with barrack compositions, handing unit spawning.", color: "0 0 255 255")]
class SCR_DefenderSpawnerComponentClass: SCR_SlotServiceComponentClass
{
	[Attribute(defvalue: "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, desc: "Default waypoint prefab", "et", category: "Defender Spawner")]
	ResourceName m_sDefaultWaypointPrefab;
};

//------------------------------------------------------------------------------------------------
//! Service providing group of defenders defined in faction. Requires SCR_EnableDefenderAction on ActionManager for players to manage its functionality.
class SCR_DefenderSpawnerComponent: SCR_SlotServiceComponent
{	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Allowed labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAllowedLabels;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Ignored labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIgnoredLabels;
	
	[Attribute("0.1", UIWidgets.Auto, category: "Defender Spawner", "Time between unit respawns")]
	protected float m_fRespawnDelay;
	
	[Attribute("1", UIWidgets.CheckBox, category: "Defender Spawner", "Enable spawning"), RplProp()]
	protected bool m_bEnableSpawning;
	
	[Attribute("-1", desc: "Disabled if -1, Used for spawn entity costs.", category: "Defender Spawner"), RplProp()]
	protected float m_fCustomSupplies;
	
	[Attribute("100", category: "Defender Spawner", desc: "Maximum range for bunkers to be used.")]
	protected float m_fSearchDistance;

	protected RplComponent m_RplComponent;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent; //TODO: Temporary until supply sandbox rework
	protected float m_fNextRespawnTime;
	protected SCR_EntityCatalogEntry m_GroupEntry;
	protected SCR_AIGroup m_AIgroup;
	protected AIWaypoint m_Waypoint;
	protected int m_iDespawnedGroupMembers;
	protected SCR_Faction m_CurrentFaction;
	protected ref ScriptInvoker m_OnSpawnerOwningFactionChanged = new ScriptInvoker(); //Invokes with new and old factions assigned to this spawner
	
	protected static const int SPAWN_CHECK_INTERVAL = 1000;
	protected static const int SPAWN_RADIUS_MIN = Math.Pow(500, 2);
	protected static const int SPAWN_RADIUS_MAX = Math.Pow(1000, 2);
	
	//------------------------------------------------------------------------------------------------
	SCR_Faction GetCurrentFaction()
	{
		return m_CurrentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns invoker called on faction change of spawner. Invokes with new faction to be assigned and previous faction
	ScriptInvoker GetOnFactionChanged()
	{
		if (!m_OnSpawnerOwningFactionChanged)
			m_OnSpawnerOwningFactionChanged = new ScriptInvoker();
		
		return m_OnSpawnerOwningFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign custom waypoint to be used by defender group
	void SetWaypoint(SCR_AIWaypoint wp)
	{
		if (wp)
			m_Waypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Defender group data from Entity catalog on owning faction
	protected void AssignDefenderGroupDataFromOwningFaction()
	{
		if (!m_CurrentFaction)
			return;
		
		SCR_EntityCatalog catalog = m_CurrentFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		if (!catalog)
		{
			Print("SCR_Faction Config lacks catalog of GROUP type. SCR_DefenderSpawnerComponent won't work without it", LogLevel.ERROR);
			return;
		}
		
		array<SCR_EntityCatalogEntry> entityEntries = {};
		array<typename> includedDataClasses = {};
		includedDataClasses.Insert(SCR_EntityCatalogSpawnerData); //~ The Data the entity must have
		
		catalog.GetFullFilteredEntityList(entityEntries, m_aAllowedLabels, m_aIgnoredLabels, includedDataClasses, null, false);
		//catalog.GetFullFilteredEntityListWithData(entityEntries, {SCR_FactionEntityDefenderData});
		
		//Only one entry of defender group should be used. Rest will be ditched, at least for now
		if (!entityEntries.IsEmpty())
			m_GroupEntry = entityEntries[0];	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign supply component to handle supplies of spawner. Currently uses Campaign specific supplies (temporarily)
	void AssignSupplyComponent(notnull SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SupplyComponent = supplyComp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get supply component assigned to spawner, temporary
	
	SCR_CampaignSuppliesComponent GetSpawnerSupplyComponent()
	{
		return m_SupplyComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get available supplies
	float GetSpawnerSupplies()
	{
		if (!m_SupplyComponent)
			return m_fCustomSupplies;
		
		return m_SupplyComponent.GetSupplies();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the spawner supplies value
	void AddSupplies(float value)
	{
		if (m_SupplyComponent)
		{
			m_SupplyComponent.AddSupplies(value);
			return;
		}
		
		m_fCustomSupplies = m_fCustomSupplies + value;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if spawner is currently activated
	bool IsSpawningEnabled()
	{
		return m_bEnableSpawning;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable spawning of defender groups
	//! \param enable bool to enable or disable spawning 
	//! \param playerID id of player using this action
	void EnableSpawning(bool enable, int playerID)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!playerController)
			return;
		
		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_CurrentFaction)
			return;
		
		m_bEnableSpawning = enable;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn Units with given ResourceName and assign it to group
	protected void SpawnUnit(ResourceName unitResource, bool consumeSupplies = true)
	{
		if (IsProxy())
			return;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;
		
		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;
		
		int spawnCost;
		
		if (m_AIgroup.m_aUnitPrefabSlots.Count() > 0)
			spawnCost = entityData.GetSupplyCost() / m_AIgroup.m_aUnitPrefabSlots.Count();
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(spawnParams.Transform);
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_AIgroup.GetOrigin();
		
		Resource res = Resource.Load(unitResource);
		
		int index = m_AIgroup.GetAgentsCount();
		
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(m_AIgroup.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		
		if (formationDefinition)		
			params.Transform[3] = m_AIgroup.CoordToParent(formationDefinition.GetOffsetPosition(index));
		else
			params.Transform[3] = m_AIgroup.CoordToParent(Vector(index, 0, 0));
		
		vector angles = Math3D.MatrixToAngles(params.Transform);
		angles[0] = GetOwner().GetAngles()[1];
		Math3D.AnglesToMatrix(angles, params.Transform);
		
		IEntity ai = GetGame().SpawnEntityPrefab(res, null, spawnParams);
		if (!ai)
			return;
		
		if (consumeSupplies)
			AddSupplies(-spawnCost);
		
		m_AIgroup.AddAIEntityToGroup(AIAgent.Cast(ai), index);
		m_AIgroup.AddAgentFromControlledEntity(ai);
		
		AIControlComponent control = AIControlComponent.Cast(ai.FindComponent(AIControlComponent));
		if (control)
			control.ActivateAI();
	}
	
	//------------------------------------------------------------------------------------------------
	//!Returns free slot
	SCR_EntitySpawnerSlotComponent GetFreeSlot()
	{	
		foreach (SCR_EntitySpawnerSlotComponent slot : m_aChildSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}
		
		foreach (SCR_EntitySpawnerSlotComponent slot : m_aNearSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refills AI's in defender group
	protected void ReinforceGroup()
	{
		array<ResourceName> unitPrefabs = {};
		unitPrefabs.Copy(m_AIgroup.m_aUnitPrefabSlots);

		array<AIAgent> outAgents = {};
		m_AIgroup.GetAgents(outAgents);
			
		foreach (AIAgent agent : outAgents)
		{
			IEntity controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity)
				return;
			
			EntityPrefabData prefabData = controlledEntity.GetPrefabData();
			if (!prefabData)
				return;
			
			ResourceName aiResource = prefabData.GetPrefabName();	
			unitPrefabs.RemoveItem(aiResource);
		}
			
		foreach (ResourceName unitPrefab : unitPrefabs)
		{	
			SpawnUnit(unitPrefab);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn defender AI group from faction prefab. Also handles price per spawned unit. 
	//! Should some units be despawned because of player distance, spawn price will be reduced by it
	protected void SpawnGroup()
	{
		if (IsProxy())
			return;
		
		if (!m_GroupEntry)
			return;
		
		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;
		
		float supplies = GetSpawnerSupplies();
		int spawnCost = entityData.GetSupplyCost();
		if (supplies < entityData.GetSupplyCost() && supplies != -1)
			return;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(params.Transform);
		
		Resource res = Resource.Load(m_GroupEntry.GetPrefab());
		if (!res.IsValid())
    		return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		if (!group || group.m_aUnitPrefabSlots.Count() == 0)
			return;
		
		if (m_iDespawnedGroupMembers > 0)
		{
			int priceReduction = (spawnCost / group.m_aUnitPrefabSlots.Count()) * m_iDespawnedGroupMembers;
			spawnCost -= priceReduction;
			m_iDespawnedGroupMembers = 0;
		}
		
		if (supplies != -1)
			AddSupplies(-spawnCost);
		
		m_AIgroup = group;
		
		//waypoint handling
		if (m_Waypoint)
		{
			group.AddWaypoint(m_Waypoint);
			return;
		}
		
		vector spawnposMat[4];
		Resource wpRes;
		SCR_AIWaypoint wp;
		
		SCR_EntityLabelPointComponent rallyPoint = slot.GetRallyPoint();
		if (rallyPoint)
		{
			spawnposMat[3] = rallyPoint.GetOwner().GetOrigin();
			Math3D.MatrixMultiply4(params.Transform, spawnposMat, params.Transform);
		}
		
		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;
		
		wpRes = Resource.Load(prefabData.m_sDefaultWaypointPrefab);
		if (!wpRes.IsValid())
    		return;

		wp = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, params));
		if (!wp)
			return;
		
		group.AddWaypoint(wp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if there is player around barracks
	protected bool PlayerDistanceCheck()
	{
		array<int> players = {};
		int playersCount = GetGame().GetPlayerManager().GetPlayers(players);
		
		float spawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MAX, SPAWN_RADIUS_MIN);
		float despawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MIN, SPAWN_RADIUS_MAX);
		IEntity playerEntity;
		float dist;
		vector origin = GetOwner().GetOrigin();
		
		for (int i = 0; i < playersCount; i++)
		{
			playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[i]);
			
			if (!playerEntity)
				continue;

			dist = vector.DistanceSq(playerEntity.GetOrigin(), origin);
			
			if (dist < spawnDistanceSq)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Manages automatic group spawning
	protected void HandleGroup()
	{
		float replicationTime = Replication.Time();
		
		bool distanceCheck = PlayerDistanceCheck();
		
		if (m_bEnableSpawning && distanceCheck && (m_iDespawnedGroupMembers > 0 || (replicationTime > m_fNextRespawnTime)))
		{
			m_fNextRespawnTime = replicationTime + (m_fRespawnDelay * 1000);
			
			if (m_AIgroup && m_AIgroup.m_aUnitPrefabSlots.Count() != m_AIgroup.GetAgentsCount())
				ReinforceGroup();
		
			if (!m_AIgroup)
				SpawnGroup();
		}
		else if (!distanceCheck && m_AIgroup)
		{
			m_iDespawnedGroupMembers = m_AIgroup.GetAgentsCount();
			SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnFactionChanged(Faction faction)
	{
		SCR_Faction newFaction = SCR_Faction.Cast(faction);
		
		m_GroupEntry = null;
		m_AIgroup = null;
		
		SCR_Faction oldFaction = m_CurrentFaction;
		m_CurrentFaction = newFaction;
		
		AssignDefenderGroupDataFromOwningFaction();
		m_OnSpawnerOwningFactionChanged.Invoke(newFaction, oldFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignInitialFaction()
	{
		if (!m_FactionControl)
			return;
		
		Faction faction = m_FactionControl.GetAffiliatedFaction();
		if (!faction)
			faction = m_FactionControl.GetDefaultAffiliatedFaction();
		
		m_CurrentFaction = SCR_Faction.Cast(faction);
		if (!m_CurrentFaction)
			m_CurrentFaction = SCR_Faction.Cast(m_FactionControl.GetDefaultAffiliatedFaction());

		if (m_CurrentFaction)
			AssignDefenderGroupDataFromOwningFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		super.EOnInit(owner);
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		AssignInitialFaction();
		
		if (IsProxy())
			return;
		
		//Setup group handling (spawning and refilling). This delay also prevents potential JIP replication error on clients
		GetGame().GetCallqueue().CallLater(HandleGroup, SPAWN_CHECK_INTERVAL, true);
	}
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DefenderSpawnerComponent()
	{
		if (m_AIgroup)
			SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);
		
		GetGame().GetCallqueue().Remove(HandleGroup);
	}
}