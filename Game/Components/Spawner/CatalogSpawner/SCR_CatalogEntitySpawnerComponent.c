[EntityEditorProps(category: "GameScripted/Components", description: "Allows player to spawn entities configured in EntityCatalogs, usually found on factions. Requires SCR_EntitySpawnerSlotComponent in vicinity.", color: "0 0 255 255")]
class SCR_CatalogEntitySpawnerComponentClass : SCR_SlotServiceComponentClass
{
	[Attribute(defvalue: "{56EBF5038622AC95}Assets/Conflict/CanBuild.emat", params: "emat", desc: "Material used on entity previews, visible to local players only", category: "Entity Spawner")]
	ResourceName m_sPreviewEntityMaterial;

	[Attribute(defvalue :"{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", UIWidgets.ResourceNamePicker, "Default group to be initially assigned to created units", "et", category: "Entity Spawner")]
	protected ResourceName m_sDefaultGroupPrefab;

	[Attribute(defvalue: "{FFF9518F73279473}PrefabsEditable/Auto/AI/Waypoints/E_AIWaypoint_Move.et", UIWidgets.ResourceNamePicker, "Defend waypoint prefab", "et", category: "Entity Spawner")]
	protected ResourceName m_sDefaultWaypointPrefab;

	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultGroupPrefab()
	{
		return m_sDefaultGroupPrefab;
	}
};

//------------------------------------------------------------------------------------------------
//! Component allowing user to request spawning entities using asset catalogs
//! Requires ActionManager with enough SCR_CatalogSpawnerUserAction attached to it (they cannot be generated through script) and SCR_EntitySlotComponents in hiearchy or vicinity of owner
class SCR_CatalogEntitySpawnerComponent : SCR_SlotServiceComponent
{
	[Attribute(category: "Catalog Parameters", desc: "Type of entity catalogs that will be allowed on this spawner.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEntityCatalogType))]
	protected ref array<EEntityCatalogType> m_aCatalogTypes;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Catalog Parameters", desc: "Allowed labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAllowedLabels;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Catalog Parameters", desc: "Ignored labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIgnoredLabels;
	
	[Attribute(defvalue: "1", desc: "If true, Spawner will require from entities that they have all allowed labels.", category: "Catalog Parameters")]
	protected bool m_bNeedAllLabels;

	[Attribute(desc: "Enables supplies usage.", category: "Supplies")]
	protected bool m_bSuppliesConsumptionEnabled;

	[Attribute(params: "0 inf", desc: "Custom supplies value.", category: "Supplies")]
	protected int m_iCustomSupplies;

	protected ActionsManagerComponent m_ActionManager;
	protected IEntity m_SpawnedEntity;
	protected SCR_PrefabPreviewEntity m_PreviewEntity;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent; //TODO: Temporary until supply sandbox rework
	protected RplComponent m_RplComponent;
	protected SCR_Faction m_CurrentFaction;

	protected ref ScriptInvoker m_OnEntitySpawned; //~ Sends Spawned IEntity
	protected ref ScriptInvoker m_OnSpawnerSuppliesChanged; //~ Sends Spawned prev and new spawn supplies
	protected ref ScriptInvoker m_OnSpawnerOwningFactionChanged; //Invokes with new and old factions assigned to this spawner

	static const int SLOT_CHECK_INTERVAL = 60;

	protected ref array<SCR_EntityCatalogEntry> m_aAssetList = {};
	protected ref map<SCR_AIGroup, IEntity> m_mGroupsToAssign;
	protected ref map<AIWaypoint, SCR_AIGroup> m_mGroupWaypoints;

	//arrays used for UI Slot checks
	protected ref map<SCR_EntitySpawnerSlotComponent, float> m_mKnownFreeSlots;
	protected ref map<SCR_EntitySpawnerSlotComponent, float> m_mKnownOccupiedSlots;

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	bool IsSuppliesConsumptionEnabled()
	{
		return m_bSuppliesConsumptionEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns invoker called on faction change of spawner. Invokes with new assigned faction and previous faction
	ScriptInvoker GetOnFactionChanged()
	{
		if (!m_OnSpawnerOwningFactionChanged)
			m_OnSpawnerOwningFactionChanged = new ScriptInvoker();

		return m_OnSpawnerOwningFactionChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_Faction GetOwningFaction()
	{
		return m_CurrentFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if user is elibigle for requesting specified entity
	//! \param entityEntry - entity entry of item to be checked
	//! \param user - controlled entity of requester
	bool RankCheck(notnull SCR_EntityCatalogEntry entityEntry, notnull IEntity user)
	{
		if (!entityEntry)
			return false;

		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return false;

		if (campaign.CanRequestWithoutRank())
			return true;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId == 0)
			return false;
		
		IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return false;

		SCR_ECharacterRank rank;

		if (IsProxy())
		{
			rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		}
		else
		{
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
			if (!campaignNetworkComponent)
				return false;

			rank = factionManager.GetRankByXP(campaignNetworkComponent.GetPlayerXP());
		}

		//Check if the player has high enough rank
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		return spawnerData && spawnerData.HasRequiredRank(rank);
	}

	//------------------------------------------------------------------------------------------------
	//! Set Asset Catalog from currently owning faction
	void SetCurrentFactionCatalog()
	{
		if (m_aCatalogTypes.IsEmpty() || !m_CurrentFaction)
			return;

		m_aAssetList.Clear();

		SCR_EntityCatalog catalog;
		foreach (EEntityCatalogType catalogType : m_aCatalogTypes)
		{
			catalog = m_CurrentFaction.GetFactionEntityCatalogOfType(catalogType);
			if (!catalog)
				continue;

			AddAssetsFromCatalog(catalog);
		}

		AssignUserActions();
	}

	//------------------------------------------------------------------------------------------------
	//! Add Assets from entityCatalog.
	//! \param entityCatalog Entity catalog which should be added
	//! \param overwriteOld IF true, old entities are overwriten by new ones, thus removing their availability
	void AddAssetsFromCatalog(notnull SCR_EntityCatalog entityCatalog, bool overwriteOld = false)
	{
		array<SCR_EntityCatalogEntry> newAssets = {};
		array<typename> includedDataClasses = {};
		includedDataClasses.Insert(SCR_EntityCatalogSpawnerData); //~ The Data the entity must have

		entityCatalog.GetFullFilteredEntityList(newAssets, m_aAllowedLabels, m_aIgnoredLabels, includedDataClasses, null, m_bNeedAllLabels);

		if (overwriteOld)
			m_aAssetList = newAssets;
		else
			m_aAssetList.InsertAll(newAssets);
	}

	//------------------------------------------------------------------------------------------------
	//! Obtain Faction entity from AssetList on specific index
	SCR_EntityCatalogEntry GetEntryAtIndex(int index)
	{
		if (!m_aAssetList || !m_aAssetList.IsIndexValid(index))
			return null;

		return m_aAssetList[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Fills SCR_CatalogSpawnerUserAction actions on ActionManager. Function should be called always, when Asset list is changed.
	protected void AssignUserActions()
	{
		if (!m_ActionManager)
			return;

		if (m_aAssetList.IsEmpty())
			return;

		array<BaseUserAction> userActions = {};
		m_ActionManager.GetActionsList(userActions);

		if (userActions.Count() < m_aAssetList.Count())
			Print("There is not enough of SCR_CatalogSpawnerUserAction attached to actionManager. Some assets won't appear.", LogLevel.WARNING);

		SCR_CatalogSpawnerUserAction spawnerUserAction;

		foreach (int i, BaseUserAction userAction : userActions)
		{
			spawnerUserAction = SCR_CatalogSpawnerUserAction.Cast(userAction);
			if (!spawnerUserAction)
				continue;

			if (m_aAssetList.IsIndexValid(i))
				spawnerUserAction.SetSpawnerData(m_aAssetList[i]);
			else
				spawnerUserAction.SetSpawnerData(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Assign supply component to handle supplies of spawner. Currently uses Campaign specific supplies (temporarily)
	void AssignSupplyComponent(notnull SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SupplyComponent = supplyComp;
	}

	//------------------------------------------------------------------------------------------------
	//! Manualy clears known slots, thus forcing Spawner to do otherwise ignored checks
	void ClearKnownSlots()
	{
		if (m_mKnownFreeSlots)
			m_mKnownFreeSlots.Clear();

		if (m_mKnownOccupiedSlots)
			m_mKnownOccupiedSlots.Clear();
	}
	//------------------------------------------------------------------------------------------------
	//! Set slot as occupied
	void AddKnownOccupiedSlot(notnull SCR_EntitySpawnerSlotComponent slot)
	{
		if (!m_mKnownOccupiedSlots)
			m_mKnownOccupiedSlots = new map<SCR_EntitySpawnerSlotComponent, float>();

		m_mKnownOccupiedSlots.Set(slot, Replication.Time());
	}

	//------------------------------------------------------------------------------------------------
	//! Update currently known occupied slots.
	//! If certain amount of time passed since last slot check, it will be removed from known slots and checked again next time it is needed.
	protected void UpdateOccupiedSlots(out array<SCR_EntitySpawnerSlotComponent> occupiedSlots)
	{
		if (!m_mKnownOccupiedSlots)
			return;

		occupiedSlots = {};

		foreach (SCR_EntitySpawnerSlotComponent slot, float timestamp : m_mKnownOccupiedSlots)
		{
			if (Replication.Time() - timestamp > (SLOT_CHECK_INTERVAL * 100))
				m_mKnownOccupiedSlots.Remove(slot);
			else
				occupiedSlots.Insert(slot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns last known suitable free slot. Used to reduce amount of redundant slot checks and save performance.
	protected SCR_EntitySpawnerSlotComponent GetLastKnownSuitableSlot(notnull SCR_EntityCatalogSpawnerData spawnerData)
	{
		if (!m_mKnownFreeSlots)
			return null;

		foreach (SCR_EntitySpawnerSlotComponent slot, float timestamp : m_mKnownFreeSlots)
		{
			if (Replication.Time() - timestamp > (SLOT_CHECK_INTERVAL * 100))
				m_mKnownFreeSlots.Remove(slot);
			else if (spawnerData.CanSpawnInSlot(slot.GetSlotType()))
				return slot;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Used in GetFreeSlot to obtain free slot from inserted array.
	//! \param slots Array containing slots that will be checked
	//! \param spawnerData Data containing parameters needed for free slot (slotType, for example)
	protected SCR_EntitySpawnerSlotComponent GetFreeSlotFromArray(notnull array<SCR_EntitySpawnerSlotComponent> slots, notnull SCR_EntityCatalogSpawnerData spawnerData, notnull array<SCR_EntitySpawnerSlotComponent> occupiedSlots)
	{
		foreach (SCR_EntitySpawnerSlotComponent slot : slots)
		{
			if (!spawnerData.CanSpawnInSlot(slot.GetSlotType()) || occupiedSlots.Contains(slot))
				continue;

			if (!slot.IsOccupied())
			{
				if (!m_mKnownFreeSlots)
					m_mKnownFreeSlots = new map<SCR_EntitySpawnerSlotComponent, float>();

				m_mKnownFreeSlots.Insert(slot, Replication.Time());

				return slot;
			}

			//Add to array of recenly found occupied slots
			AddKnownOccupiedSlot(slot);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!Returns free slot suitable for requested Entity
	//! \param spawnerData SCR_EntityCatalogSpawnerData found on Catalog Entry
	SCR_EntitySpawnerSlotComponent GetFreeSlot(notnull SCR_EntityCatalogSpawnerData spawnerData)
	{
		// First go through recently checked slots to prevent redundant checks. Should be used only for Interaction visualisation.
		SCR_EntitySpawnerSlotComponent freeSlot = GetLastKnownSuitableSlot(spawnerData);
		if (freeSlot)
			return freeSlot;

		array<SCR_EntitySpawnerSlotComponent> occupiedSlots = {};
		UpdateOccupiedSlots(occupiedSlots);

		// Second check for slots in hiearchy, If no child slot available, look for nearby slots
		freeSlot = GetFreeSlotFromArray(m_aChildSlots, spawnerData, occupiedSlots);
		if (!freeSlot)
			freeSlot = GetFreeSlotFromArray(m_aNearSlots, spawnerData, occupiedSlots);

		return freeSlot;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Initiate spawn. Called through RPC from SCR_SpawnerRequestComponent on CharacterComponent
	\param entityEntry entity entry to be spawned
	\param userId Id of user requesting spawn
	\param slot Slot on which user requests spawn. As last check for empty position is done by PerformSpawn itselt, this might be ignored, should slot be already occupied.
	*/
	void InitiateSpawn(notnull SCR_EntityCatalogEntry entityEntry, int userId, SCR_EntitySpawnerSlotComponent slot)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(userId);
		if (!user)
			return;

		int supplies = GetSpawnerSupplies();
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!spawnerData)
			return;
		
		if (m_bSuppliesConsumptionEnabled && supplies < spawnerData.GetSupplyCost())
			return;

		if (GetRequestState(entityEntry, user) == SCR_EEntityRequestStatus.CAN_SPAWN)
			PerformSpawn(entityEntry, user, slot);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Returns SCR_EEntityRequestStatus for request of selected entity with optional parameter of user requesting it.
	\param entityEntry entityEntry in catalog to be spawned
	\param user User requesting spawn
	*/
	SCR_EEntityRequestStatus GetRequestState(notnull SCR_EntityCatalogEntry entityEntry, IEntity user = null)
	{
		if (!m_aAssetList || !m_aAssetList.Contains(entityEntry))
			return SCR_EEntityRequestStatus.NOT_AVAILABLE;

		SCR_EntityCatalogSpawnerData entitySpawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		//Supply consumption specific states
		if (m_bSuppliesConsumptionEnabled)
		{
			if (m_SupplyComponent && entitySpawnerData && (entitySpawnerData.GetSupplyCost() > m_SupplyComponent.GetSupplies()))
				return SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES;
			else if (!m_SupplyComponent && entitySpawnerData && (entitySpawnerData.GetSupplyCost() > m_iCustomSupplies))
				return SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES;
		}

		//Do not show anything, if user is of another faction than spawner
		if (GetRequesterFaction(user) != m_CurrentFaction)
			return SCR_EEntityRequestStatus.NOT_AVAILABLE;

		// Campaign dependent ranking and cooldowns. To be replaced once stand-alone rank system is present
		if (SCR_GameModeCampaignMP.GetInstance())
		{
			if (!RankCheck(entityEntry, user))
				return SCR_EEntityRequestStatus.RANK_LOW;

			if (!CooldownCheck(user))
				return SCR_EEntityRequestStatus.COOLDOWN;
		}

		// Additional states to be returned if entity Entry is of Character type
		if (entityEntry.GetCatalogParent().GetCatalogType() == EEntityCatalogType.CHARACTER)
		{
			SCR_PlayerController controller = GetPlayerControllerFromEntity(user);
			if (!controller)
				return SCR_EEntityRequestStatus.NOT_AVAILABLE;

			SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
			if (!groupController || !groupController.IsPlayerLeaderOwnGroup())
				return SCR_EEntityRequestStatus.REQUESTER_NOT_GROUPLEADER;
		}

		return SCR_EEntityRequestStatus.CAN_SPAWN;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when AI finishes initial WP (making it move away from spawning position, before joining player squad)
	protected void OnGroupWaypointFinished(notnull AIWaypoint wp)
	{
		if (!m_mGroupWaypoints || !m_mGroupsToAssign)
			return;

		SCR_AIGroup group = m_mGroupWaypoints.Get(wp);
		if (!group)
			return;

		IEntity player = m_mGroupsToAssign.Get(group);
		if (!player)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		SCR_ChimeraCharacter aiCharacter;
		foreach (AIAgent agent : agents)
		{
			aiCharacter = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!aiCharacter)
				continue;

			AddAISoldierToPlayerGroup(aiCharacter, player);
		}

		//Delete old entries or empty maps
		m_mGroupWaypoints.Remove(wp);
		if (m_mGroupWaypoints.IsEmpty())
			delete m_mGroupWaypoints;

		m_mGroupsToAssign.Remove(group);
		if (m_mGroupsToAssign.IsEmpty())
			delete m_mGroupsToAssign;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddAISoldierToPlayerGroup(notnull SCR_ChimeraCharacter ai, notnull IEntity user)
	{
		SCR_PlayerController controller = GetPlayerControllerFromEntity(user);
		if (!controller)
			return;

		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent)
			return;

		groupComponent.RequestAddAIAgent(ai, controller.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	SCR_Faction GetRequesterFaction(notnull IEntity user)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(user);
		if (!player)
			return null;

		return SCR_Faction.Cast(player.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Gets script invoker on entity spawns.
	Invoker will send over spawned IEntity
	\return ScriptInvoker Event when entity is spawned
	*/
	ScriptInvoker GetOnEntitySpawned()
	{
		if (!m_OnEntitySpawned)
			m_OnEntitySpawned = new ScriptInvoker();

		return m_OnEntitySpawned;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Gets script invoker on supplies changes.
	Invoker will send over previous supply amount and changed one
	\return ScriptInvoker Event when entity is spawned
	*/
	ScriptInvoker GetOnSpawnerSuppliesChanged()
	{
		if (!m_OnSpawnerSuppliesChanged)
			m_OnSpawnerSuppliesChanged = new ScriptInvoker();

		return m_OnSpawnerSuppliesChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerController GetPlayerControllerFromEntity(notnull IEntity userEntity)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(userEntity);

		return SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

	}

	//------------------------------------------------------------------------------------------------
	// Send notification to player requesting spawn
	protected void SendNotification(int msgId, notnull IEntity user, int assetId = -1, int catalogType = -1)
	{
		SCR_PlayerController playerController = GetPlayerControllerFromEntity(user);
		if (!playerController)
			return;

		SCR_SpawnerRequestComponent reqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!reqComponent)
			return;

		reqComponent.SendPlayerFeedback(msgId, assetId, catalogType);
	}

	//------------------------------------------------------------------------------------------------
	//! Used to create local "preview" model on specified slot. Used to visualise what entity is player requesting and position where it will appear
	//! \param spawnData - SCR_EntityCatalogEntry containing information about entity (prefab data is required)
	//! \param slot - Slot on which preview should appear
	void CreatePreviewEntity(notnull SCR_EntityCatalogEntry spawnData, notnull SCR_EntitySpawnerSlotComponent slot)
	{
		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		//This will delete any existing preview entity
		DeletePreviewEntity();

		if (!m_aAssetList || m_aAssetList.IsEmpty() || !m_aAssetList.Contains(spawnData))
			return;

		Resource resource = Resource.Load(spawnData.GetPrefab());

		if (!resource || !resource.IsValid())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		slot.GetOwner().GetTransform(params.Transform);
		m_PreviewEntity = SCR_PrefabPreviewEntity.Cast(SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(resource, "SCR_PrefabPreviewEntity", GetOwner().GetWorld(), params, prefabData.m_sPreviewEntityMaterial));
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes existing Preview entity at spawner
	void DeletePreviewEntity()
	{
		if (m_PreviewEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns existing preview entity
	SCR_PrefabPreviewEntity GetPreviewEntity()
	{
		return m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Spawn the actual entity
	\param index of entity
	\param entity initiating the spawn
	\param slot optional slot to be used for spawning
	*/
	protected void PerformSpawn(notnull SCR_EntityCatalogEntry entityEntry, IEntity user = null, SCR_EntitySpawnerSlotComponent preferredSlot = null)
	{
		if (IsProxy())
			return;

		m_SpawnedEntity = null;

		ResourceName spawnResource = entityEntry.GetPrefab();
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		Resource resource = Resource.Load(spawnResource);
		if (!resource || !resource.IsValid())
		{
			Print("CatalogEntitySpawnerComponent - PerformSpawn cannot spawn new entity without valid resource. Used SCR_EntityCatalogEntry is probably set incorrectly", LogLevel.ERROR);
			return;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		//Slot from preferredSlot parameter is checked first, should it be occupied, new slot will be looked for (if there is any)
		SCR_EntitySpawnerSlotComponent slot;
		if (preferredSlot && spawnerData.CanSpawnInSlot(preferredSlot.GetSlotType()) && !preferredSlot.IsOccupied())
		{
			slot = preferredSlot;
		}
		else
		{
			ClearKnownSlots();
			slot = GetFreeSlot(spawnerData)
		}

		if (!slot)
		{
			SendNotification(SCR_EEntityRequestStatus.NOT_ENOUGH_SPACE, user);
			return;
		}

		preferredSlot = slot;
		//Obtain transformation of selected slot
		slot.GetOwner().GetTransform(params.Transform);

		m_SpawnedEntity = GetGame().SpawnEntityPrefab(resource, GetOwner().GetWorld(), params);
		if (!m_SpawnedEntity)
			return;

		if (m_bSuppliesConsumptionEnabled)
			AddSpawnerSupplies(-spawnerData.GetSupplyCost());

		if (m_OnEntitySpawned)
			m_OnEntitySpawned.Invoke(m_SpawnedEntity);

		//Send notification to player, whom requested entity
		SCR_EntityCatalog parentCatalog = entityEntry.GetCatalogParent();
		if (parentCatalog)
		{
			array<SCR_EntityCatalogEntry> entityList = {};
			parentCatalog.GetEntityList(entityList);
			int assetIndex = entityList.Find(entityEntry);
			
			if (assetIndex > -1)
				SendNotification(0, user, entityList.Find(entityEntry), parentCatalog.GetCatalogType());
		}

		//Called, if spawned entity is vehicle
		if (m_SpawnedEntity.IsInherited(Vehicle))
		{
			Physics physicsComponent = m_SpawnedEntity.GetPhysics();
			if (physicsComponent)
				physicsComponent.SetVelocity("0 -0.1 0"); // Make the entity copy the terrain properly

			//Temporary
			SCR_GameModeCampaignMP gamemode = SCR_GameModeCampaignMP.GetInstance();
			if (gamemode)
				gamemode.VehicleSpawned(m_SpawnedEntity, user, GetOwningFaction());
		}

		//Called, if spawned entity is character
		if (m_SpawnedEntity.IsInherited(SCR_ChimeraCharacter))
			OnChimeraCharacterSpawned(SCR_ChimeraCharacter.Cast(m_SpawnedEntity), user, slot.GetRallyPoint());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get supply component assigned to spawner
	\return SCR_CampaignSuppliesComponent spawn supplies
	*/
	SCR_CampaignSuppliesComponent GetSpawnerSupplyComponent()
	{
		return m_SupplyComponent;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get supplies left for spawner
	\return float spawn supplies
	*/
	float GetSpawnerSupplies()
	{
		if (!m_SupplyComponent)
			return m_iCustomSupplies;

		return m_SupplyComponent.GetSupplies();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Add or reduce (by entering negative value) supplies available to spawner
	\param value amount of supplies to be added/reduced
	*/
	void AddSpawnerSupplies(float supplies)
	{
		if (m_SupplyComponent)
		{
			m_SupplyComponent.AddSupplies(supplies);

			if (m_OnSpawnerSuppliesChanged)
				m_OnSpawnerSuppliesChanged.Invoke(m_SupplyComponent.GetSupplies());

			return;
		}
		
		m_iCustomSupplies += supplies;
		Replication.BumpMe();

		if (m_OnSpawnerSuppliesChanged)
			m_OnSpawnerSuppliesChanged.Invoke(m_iCustomSupplies);
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
			SetCurrentFactionCatalog();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns false if player spawned too recently
	protected bool CooldownCheck(notnull IEntity user)
	{
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (userId == 0)
			return false;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(userId));
		if (!playerController)
			return false;

		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!networkComponent)
			return false;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager)
			return false;

		SCR_ECharacterRank rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		float timeout = networkComponent.GetLastRequestTimestamp() + (float)factionManager.GetRankRequestCooldown(rank);

		return timeout < Replication.Time();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnFactionChanged(Faction faction)
	{
		SCR_Faction newFaction = SCR_Faction.Cast(faction);
		if (!newFaction)
			return;
		
		SCR_Faction oldFaction = m_CurrentFaction;
		m_CurrentFaction = newFaction;

		SetCurrentFactionCatalog();

		if (m_OnSpawnerOwningFactionChanged)
			m_OnSpawnerOwningFactionChanged.Invoke(newFaction, oldFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from PerformSpawn, if spawned entity is ChimeraCharacter
	protected void OnChimeraCharacterSpawned(notnull SCR_ChimeraCharacter ai, notnull IEntity user, SCR_EntityLabelPointComponent rallyPoint = null)
	{
		AIControlComponent control = AIControlComponent.Cast(ai.FindComponent(AIControlComponent));
		if (!control)
			return;
		
		control.ActivateAI();

		// If other type of slot is used, thus without default waypoint, AI unit will be added directly into player group, not going anywhere before that.
		if (!rallyPoint)
		{
			AddAISoldierToPlayerGroup(ai, user);
			return;
		}

		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		SCR_AIWaypoint wp = CreateRallyPointWaypoint(rallyPoint);

		AIAgent agent = control.GetAIAgent();
		if (!agent)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(agent.GetParentGroup());
		if (!group)
		{
			Resource res = Resource.Load(prefabData.GetDefaultGroupPrefab());
			group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld()));
			if (!group)
				return;
		}

		group.AddAgent(agent);
		group.AddWaypoint(wp);

		if (!m_mGroupWaypoints)
			m_mGroupWaypoints = new map<AIWaypoint, SCR_AIGroup>();

		m_mGroupWaypoints.Insert(wp, group);

		group.GetOnWaypointCompleted().Insert(OnGroupWaypointFinished);

		if (!m_mGroupsToAssign)
			m_mGroupsToAssign = new map<SCR_AIGroup, IEntity>();

		m_mGroupsToAssign.Insert(group, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns waypoint created as rally point for spawned unit
	protected SCR_AIWaypoint CreateRallyPointWaypoint(notnull SCR_EntityLabelPointComponent rallyPoint)
	{
		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		EntitySpawnParams wpParams = new EntitySpawnParams();
		wpParams.TransformMode = ETransformMode.WORLD;
		rallyPoint.GetOwner().GetTransform(wpParams.Transform);

		Resource wpRes = Resource.Load(prefabData.GetDefaultWaypointPrefab());
		if (!wpRes.IsValid())
			return null;

		return SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, wpParams));
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
		{
			Print("SCR_CatalogEntitySpawnerComponent requires RplComponent for its functionality!", LogLevel.WARNING);
			return;
		}

		m_ActionManager = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!m_ActionManager)
			Print("No Action Manager detected on owner of Spawner Component!", LogLevel.WARNING);

		AssignInitialFaction();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EEntityRequestStatus
{
	NOT_ENOUGH_SUPPLIES = 1,
	NOT_ENOUGH_SPACE = 2,
	RANK_LOW = 3,
	COOLDOWN = 4,
	REQUESTER_NOT_GROUPLEADER = 5,
	GROUP_FULL = 6,
	NOT_AVAILABLE = 7,
	CAN_SPAWN = 8,
	CAN_SPAWN_TRIGGER = 9
};
