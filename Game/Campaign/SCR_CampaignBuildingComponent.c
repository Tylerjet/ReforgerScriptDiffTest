[EntityEditorProps(category: "GameScripted/Campaign", description: "Component to activate building mode in base.", color: "0 0 255 255")]
class SCR_CampaignBuildingComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingComponent : ScriptComponent
{
	//! Distance (radius) in which the slots will be used for building compositions.
	[Attribute("300", UIWidgets.Slider, "Distance from player in which slots are used for building", "1 1000 1")]
	protected float m_fBuildingRadius;
	
	//! Multiplier used to calculate distance on which preview compositions are deleted.
	[Attribute("1.2", UIWidgets.Slider, "Multiplier used for deleting preview compositions", "1 10 1")] 
	protected float m_fDeletingMultiplier;
	
	//! Prefab used for creating a ghost composition
	[Attribute("", UIWidgets.ResourceNamePicker, "Preview Entity Prefab", "et")]
	protected ResourceName m_PreviewEntityPrefab;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Preview entity material", "et")]
	protected ResourceName m_PreviewMaterial;
	
	//! Prefab used for interaction with composition Preview 
	[Attribute("", UIWidgets.ResourceNamePicker, "Building Entity Prefab", "et")]
	protected ResourceName m_BuildingEntityPrefab;
			
	// array of slots
	protected ref array<SCR_SiteSlotEntity> m_aSlotEntities = new array<SCR_SiteSlotEntity>();	
	
	// array of all created preview entities
	protected ref array<SCR_BasePreviewEntity> m_aPreviewEntities;
	
	// array of all controllers of preview Entities
	protected ref map<SCR_SiteSlotEntity, IEntity> m_mControllerEntities;
	
	// array of all prefabs which can't be dismounted
	protected ref array<ResourceName> m_aUndismountablePrefabs = new array<ResourceName>();
	
	// array of all building HUD icons
	protected ref array<ImageWidget> m_aHUDIcons = new array<ImageWidget>;
	
	protected bool m_bInBuildingMode = false;
	protected bool m_bPreviewUpdateLoop = false;
	protected bool m_bAwayFromAreaCheckRunning;
	protected vector m_vBuildingAreaCenter;
	protected SCR_CampaignFaction m_OwningFaction;
	protected SCR_GameModeCampaignMP m_GameModeCampaignMP;
	protected SCR_CampaignBase m_ParentBase;
	
	// composition cost multiplier
	protected float m_fCostMultiplier = 1;
	
	// composition return cost multiplier 
	protected float m_fRefundMultiplier = 1;
	
	protected IEntity m_BuildComposition;

	// vehicle on which the action was executed
	protected IEntity m_VehicleInstance;
	protected DamageManagerComponent m_DamageManagerVehicle;
	protected bool m_bPlayerInHisSupplyTruck = false;
		
	//distance in which the list of composition is updated again (moving vehicle)
	protected const int UPDATE_DISTANCE = 10000; //100 m (value is squared) 
	
	//A time in which the update loop is running. 
	// Each x sec the loop checks the distance between the player and the location where he initiates the building mode. This one is set for a building in base.
	protected const int UPDATE_TIME_LOCATION = 4000; // 4 sec
	
	//A time in which the update loop is running. 
	//Each x sec the loop checks the distance between player and the location where he initiates the building mode. This one is set for the Truck
	protected const int UPDATE_TIME_TRUCK = 4000; // 4 sec
	protected const int SEARCH_DISTANCE = 2;
		
	// How far away from the player will the HUD icons get drawn (squared to be usable by distance measurements)
	protected const int MAX_HUD_ICON_DISTANCE = Math.Pow(100, 2);
	protected const int DEFAULT_HUD_ICON_SIZE = 64;
	
	//------------------------------------------------------------------------------------------------
	// executed locally when player dies
	private void OnDeath()
	{	
		QuitBuilding();
	}
	
	//------------------------------------------------------------------------------------------------
	// executed locally when truck (canvas with supplies) is deleted
	private void OnDeleted()
	{	
		QuitBuilding();
	}
	
	//------------------------------------------------------------------------------------------------
	// executed locally when supply truck is destroyed
	private void OnSupplyTruckDestroyed()
	{	
		EDamageState state = m_DamageManagerVehicle.GetState();
		if (state == EDamageState.DESTROYED)
		{
			QuitBuilding();
		}
	}

	//------------------------------------------------------------------------------------------------
	// executed when base is seized by enemy.
	private void OnBaseCaptured(SCR_CampaignBase base = null)
	{
		if (!base)
			return;
		
		if (base != m_ParentBase)
			return;
		
		QuitBuilding();
	}
	
	//------------------------------------------------------------------------------------------------
	private void QuitBuilding()
	{
		SetBuilding(false);
		RemovePreviewEntities();
		RemoveControllers();
		RemoveHandlers();
	}
	
	//------------------------------------------------------------------------------------------------
	// executed when the player enters the vehicle (building mode is on)
	private void OnSupplyTruckEntered(IEntity vehicle)
	{
		if (!vehicle || vehicle != m_VehicleInstance)
			return;
		
		m_bPlayerInHisSupplyTruck = true;
		StartUpdatePreviewLoop();		
	}
	
	//------------------------------------------------------------------------------------------------
	// executed when the player left the vehicle (building mode is on)
	private void OnSupplyTruckLeft(IEntity vehicle)
	{
		if (!vehicle || vehicle != m_VehicleInstance || !SCR_PlayerController.GetLocalControlledEntity())
			return;
		
		// Update center of radius where is the player building...
		m_vBuildingAreaCenter = SCR_PlayerController.GetLocalControlledEntity().GetOrigin();
		m_bPlayerInHisSupplyTruck = false;	
		StopUpdatePreviewLoop();
	}
					
	//------------------------------------------------------------------------------------------------
	void AwayFromArea(IEntity player)
	{
		// Check, which will delete preview if the player or the truck gets too far from building area. 
		if (!m_bPlayerInHisSupplyTruck && (PlayerOutOfBuildingArea(player) || m_VehicleInstance && TruckOutOfBuildingArea(m_VehicleInstance)))
		{
			//the player runs too far from the building area. Delete previews and stop ticking.
			GetGame().GetCallqueue().Remove(AwayFromArea);
			QuitBuilding();
			
			// let the system know that the loop is not running any more
			m_bAwayFromAreaCheckRunning = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateShownCompositions()
	{
		// Check that will be spawning previews around the vehicle. Compare position of vehicle with it's previous position.
		GenericEntity owner = GetOwner(); 		
		
		if (owner && vector.DistanceSq(m_vBuildingAreaCenter, owner.GetOrigin()) > UPDATE_DISTANCE)
		{
			// ToDo: Optimization - delete only the entities which are obsolete and spawn a new one.
			RemovePreviewEntities();
			RemoveControllers();
			
			// Get all slots in given distance
			GetSlotsNearby(owner, m_fBuildingRadius);
			SpawnPreviews(owner.GetParent(), SCR_PlayerController.GetLocalControlledEntity() , m_OwningFaction);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// Building previews was executed from the vehicle
	void SpawnPreviewsVehicle(notnull IEntity pOwnerEntity)
	{			
		m_VehicleInstance = pOwnerEntity.GetParent();
		if (!m_VehicleInstance)
			return;
			
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(m_VehicleInstance.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
		    return;
					
		m_OwningFaction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
		if (!m_OwningFaction)
		    return;
		
		m_DamageManagerVehicle = DamageManagerComponent.Cast(m_VehicleInstance.FindComponent(DamageManagerComponent));
		if (!m_DamageManagerVehicle)
			return;
		
		// Add invoker to delete previews when the truck is destroyed
		ScriptedHitZone zone = ScriptedHitZone.Cast(m_DamageManagerVehicle.GetDefaultHitZone());
		if (zone)
			zone.GetOnDamageStateChanged().Insert(OnSupplyTruckDestroyed);
		
		// Add invoker to delete preview when the player runs too far from vehicle (exclude situation if he is in this vehicle)		
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CompartmentAccessComponent));
		if (accessComp)
		{
			accessComp.GetOnCompartmentEntered().Insert(OnSupplyTruckEntered);
			accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
		}
		
		// Add invoker to delete previews when the truck is deleted
		SCR_CampaignSuppliesComponent supplyComponent = SCR_CampaignSuppliesComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignSuppliesComponent));
		if (supplyComponent)
			supplyComponent.m_OnSuppliesTruckDeleted.Insert(OnDeleted);
				
		// Get all slots in given distance
		GetSlotsNearby(pOwnerEntity, m_fBuildingRadius);
		SpawnPreviews(pOwnerEntity.GetParent(), SCR_PlayerController.GetLocalControlledEntity(), m_OwningFaction);
	}
		
	//------------------------------------------------------------------------------------------------
	// Building previews was executed from the base
	void SpawnPreviewsBase(notnull SCR_CampaignBase base, notnull array<SCR_SiteSlotEntity> slots)
	{	
		m_aSlotEntities = slots;
		m_OwningFaction = base.GetOwningFaction();
		m_ParentBase = base;
		// Clients
		SCR_CampaignBase.s_OnBaseOwnerChanged.Insert(OnBaseCaptured);
		// Server
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnBaseCaptured);
		SpawnPreviews(base, SCR_PlayerController.GetLocalControlledEntity(), m_OwningFaction);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnPreviews(IEntity pOwnerEntity, IEntity player, SCR_CampaignFaction faction)
	{	
		// One time. If the array is empty, fill it.
		if (m_aUndismountablePrefabs.IsEmpty())
			GetUndismountableCompositions(faction);
				
		// Invoker handling deleting composition preview on clients
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core) 
			return;
			
		core.Event_OnEntityExtendedChange.Insert(CheckSlots);
		
		// register player EH onDeath - to delete active previews and building controllers
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
		    return;
		
		characterControllerComponent.m_OnPlayerDeath.Insert(OnDeath);
		
		Color iconColor;
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(characterControllerComponent.GetCharacter().GetFaction());
		
		if (f)
			iconColor = f.GetFactionColor();
		
		for (int i = m_aSlotEntities.Count() - 1; i >= 0; i--)
		{
			// ----------------------------------------------------
			// Step 1. Get slot information about the slot
			// ----------------------------------------------------
			ResourceName resName = m_aSlotEntities[i].GetPrefabData().GetPrefabName();
			
			// Get slot data
			array<ref SCR_CampaignSlotComposition> slotDataArray = faction.GetSlotResource(GetCompositionType(resName));
			if (slotDataArray.IsEmpty())
			{
				#ifdef ENABLE_DIAG
				Print("There aren't defined a compositions for this type of the slot");
				#endif
				// ToDo: Possible optimization - don't add slots which has not assigned any composition
				m_aSlotEntities.RemoveOrdered(i);
				continue;
			}
			
			// ----------------------------------------------------
			// Step 2. Check whether this slot is a special one - for Services composition.
			// ----------------------------------------------------
			SCR_CampaignSlotComposition slotData;
			if (!m_aSlotEntities[i].IsOccupied() && m_GameModeCampaignMP.GetSlotPresetResourceName(m_aSlotEntities[i]) != ResourceName.Empty)
			{
				// Get all compositions for Services
				array<ref SCR_CampaignSlotComposition> slotDataArrayServices = faction.GetSlotResource(SCR_ESlotTypesEnum.Services);
				
				for (int y = slotDataArrayServices.Count() -1; y >= 0; y--)
				{
					if (slotDataArrayServices[y].GetResourceName() == m_GameModeCampaignMP.GetSlotPresetResourceName(m_aSlotEntities[i], faction))
					{
						slotData = slotDataArrayServices[y];
					}
				}
			}
			else 
			{
				//As a default we will spawn the 1st composition from the list of all available.
				slotData = slotDataArray[0];
			}
			
			if (!slotData)
				continue;
			
			// ----------------------------------------------------
			// Step 3. Check if the slot is already used. If so, show disassembly option
			// ----------------------------------------------------
			
			BaseWorld world = GetGame().GetWorld();
			
			if (m_aSlotEntities[i].IsOccupied())
			{				
				world.QueryEntitiesBySphere(m_aSlotEntities[i].GetOrigin(), SEARCH_DISTANCE, FindComposition, null, EQueryEntitiesFlags.ALL);
				if (!m_BuildComposition)
					continue;
								
				EntityPrefabData prefabData = m_BuildComposition.GetPrefabData();
				if (!prefabData)
					continue;
				
				ResourceName res = prefabData.GetPrefabName();			
				if (!res)
					continue;
						
				//Check if the res is on the list of "protected" compositions which can't be dismounted.
				if (m_aUndismountablePrefabs.Find(res) == -1)
					SpawnController(slotData, m_aSlotEntities[i], pOwnerEntity, null);
				
				m_BuildComposition = null;
				continue;
			}
			
			SpawnNewPreview(slotData, m_aSlotEntities[i], pOwnerEntity, color: iconColor);
		}

		// Get the player position and start ticking...
		m_vBuildingAreaCenter = SCR_PlayerController.GetLocalControlledEntity().GetOrigin();
	
		// Check if player or truck isn't out of the building area
		if (!m_bAwayFromAreaCheckRunning)
			GetGame().GetCallqueue().CallLater(AwayFromArea, UPDATE_TIME_LOCATION, true, player); 
	}
	
	//------------------------------------------------------------------------------------------------
	// Spawn a composition preview with it's controler.
	void SpawnNewPreview(SCR_CampaignSlotComposition slotData ,SCR_SiteSlotEntity slot, IEntity suppliesProvider, SCR_CampaignBuildingControllerComponent buildingController = null, Color color = null)
	{	
		// create preview
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		slot.GetWorldTransform(spawnParams.Transform);
		SCR_BasePreviewEntity previewEntity = SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(Resource.Load(slotData.GetResourceName()), m_PreviewEntityPrefab, slot.GetWorld(), spawnParams, m_PreviewMaterial);
		if (!previewEntity)
			return;
		
		// Add preview entity on the list
		m_aPreviewEntities.Insert(previewEntity);
				
		//ToDo: Enable this later also for Services
		if (!slotData.IsServiceComposition())
			UpdateCompositionOrientation(previewEntity);
	
		// if the controller is null, spawn one, if not, only update it's data
		if (!buildingController)	
		{
			SpawnController(slotData, slot, suppliesProvider, previewEntity);
		}
		else
		{
			SetControllerData(slotData, slot, buildingController, previewEntity, suppliesProvider);
		}
		
		// Spawn HUD icon
		string iconName = slotData.GetIconImage();
		
		if (!iconName.IsEmpty() && color)
		{
			Widget HUDIconRoot = GetGame().GetHUDManager().CreateLayout(m_GameModeCampaignMP.GetHUDIconLayout(), EHudLayers.BACKGROUND);
			ImageWidget newIcon = ImageWidget.Cast(HUDIconRoot.FindAnyWidget("Icon"));
			m_aHUDIcons.Insert(newIcon);	// Insert it even if it's null to maintain array order
			
			if (newIcon)
			{
				newIcon.LoadImageFromSet(0, m_GameModeCampaignMP.GetBuildingIconsImageset(), iconName);
				SetEventMask(GetOwner(), EntityEvent.FRAME);
				newIcon.SetColor(color);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Update orientation of the composition according to the terrain
	void UpdateCompositionOrientation(notnull SCR_BasePreviewEntity previewEntity)
	{
		vector previewTransform[4];
		previewEntity.GetWorldTransform(previewTransform);
		previewEntity.SetPreviewTransform(previewTransform, EEditorTransformVertical.TERRAIN, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	// Spawn only the controler
	void SpawnController(SCR_CampaignSlotComposition slotData, notnull SCR_SiteSlotEntity slot, IEntity suppliesProvider, SCR_BasePreviewEntity previewEntity)
	{
		// controller resource
		Resource resource = Resource.Load(m_BuildingEntityPrefab);
		if (!resource.IsValid())
			return;
	
		// This spawned controller needs to be a child (in heararchy) of the entity with resources - I need to be able to get the current state of supply. Ether of the base or the truck etc...)		
		EntitySpawnParams spawn_params = EntitySpawnParams();
		spawn_params.TransformMode = ETransformMode.WORLD;
				
		spawn_params.Transform[3] = slot.GetOrigin() + slot.VectorToParent(slotData.GetBuildingControllerOffset());
		spawn_params.Transform[3][1] = slotData.GetBuildingControllerOffset()[1] + GetGame().GetWorld().GetSurfaceY(spawn_params.Transform[3][0], spawn_params.Transform[3][2]);
		IEntity controller = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawn_params);

		SCR_CampaignBuildingControllerComponent buildingController = SCR_CampaignBuildingControllerComponent.Cast(controller.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!buildingController)
			return;
		
		// Create the map
		if (!m_mControllerEntities)
			m_mControllerEntities = new map<SCR_SiteSlotEntity, IEntity>;
		
		// Add controller entity on the list too.
  		m_mControllerEntities.Insert(slot,controller);
		SetControllerData(slotData, slot, buildingController, previewEntity, suppliesProvider);
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the data in building controller
	void SetControllerData(SCR_CampaignSlotComposition slotData, SCR_SiteSlotEntity slot, SCR_CampaignBuildingControllerComponent buildingController, SCR_BasePreviewEntity previewEntity, IEntity suppliesProvider)
	{
		buildingController.SetEntity(previewEntity);
		buildingController.SetData(slotData);
		buildingController.SetStartAngle(slot.GetLocalAngles());
		buildingController.SetUsedSlot(slot);
		buildingController.SetSuppliesProvider(suppliesProvider);
		buildingController.SetMarker();
		if (!buildingController.GetTrigger())
			buildingController.SpawnTrigger();
			
		// If we can get a buildingManagerEntity set the cost multipliers
		SCR_CampaignBuildingManagerEntity buildingManagerEnt = SCR_CampaignBuildingManagerEntity.GetInstance();
		if (buildingManagerEnt)
		{
			buildingController.SetCostMultiplier(buildingManagerEnt.GetCostMultiplier());
			buildingController.SetRefundMultiplier(buildingManagerEnt.GetRefundMultiplier());
		}
		
		buildingController.HasAvailableResources();
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the composition cost multiplier
	void SetCostMultiplier(float costMultiplier)
	{		
		if (!m_mControllerEntities)
			return;
		
		for (int i = m_mControllerEntities.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignBuildingControllerComponent comp = SCR_CampaignBuildingControllerComponent.Cast(m_mControllerEntities.GetElement(i).FindComponent(SCR_CampaignBuildingControllerComponent));
			comp.SetCostMultiplier(costMultiplier);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the return cost multiplier
	void SetRefundMultiplier(float refundMultiplier)
	{
		if (!m_mControllerEntities)
			return;
		
		for (int i = m_mControllerEntities.Count() - 1; i >=0 ;i--)
		{
			SCR_CampaignBuildingControllerComponent comp = SCR_CampaignBuildingControllerComponent.Cast(m_mControllerEntities.GetElement(i).FindComponent(SCR_CampaignBuildingControllerComponent));
			comp.SetRefundMultiplier(refundMultiplier);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Create an array of all ResourceNames - compositions which can't be dismounted.
	void GetUndismountableCompositions(SCR_CampaignFaction faction)
	{
		if (!faction)
			return;
		
		SCR_CampaignFaction enemyFaction = SCR_CampaignFactionManager.GetInstance().GetEnemyFaction(faction);	
		if (!enemyFaction)
			return;
		
		for (int y = ECampaignCompositionType.LAST; y >= 0; y--)
		{
			// Players compositions
			ResourceName resNameAlly = faction.GetBuildingPrefab(y);
			if (resNameAlly != ResourceName.Empty)
				m_aUndismountablePrefabs.Insert(resNameAlly);
			
			// Enemy compositions
			ResourceName resNameEnemy = enemyFaction.GetBuildingPrefab(y);
			if (resNameEnemy != ResourceName.Empty)
				m_aUndismountablePrefabs.Insert(resNameEnemy);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Compare resource name with the defined slots and if match is found, return SlotTypesEnum
	SCR_ESlotTypesEnum GetCompositionType(ResourceName resName)
	{
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		for (int i = 0; i <= SCR_ESlotTypesEnum.Services; i++ )
		{
			ResourceName name = factionManager.GetSlotsResource(i);
			
			if (resName == name)
				return i;
		}
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get all slots and insert them into the array
	protected bool FilterEntities(IEntity ent)
	{		
		SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(ent);
		
		if (!slotEnt)
		    return true;
		
		// Check if the slot is not disabled, allow occupied slots so the composition can be dismantled
		SCR_EditableEntityComponent comp = SCR_EditableEntityComponent.Cast(slotEnt.FindComponent(SCR_EditableEntityComponent));
		
		if ((comp && comp.GetVisibleSelf()) || slotEnt.IsOccupied())
			AddSlot(slotEnt);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the composition build at the slot.
	protected bool FindComposition(notnull IEntity ent)
	{					
		IEntity parent = SCR_EntityHelper.GetMainParent(ent, true);
			
		if (!parent.FindComponent(SCR_SlotCompositionComponent))
		    return true;
		
		SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(ent);
		if (slotEnt)
			return true;
		
		EntityPrefabData prefabData = ent.GetPrefabData();
		if (!prefabData)
			return true;
		
		ResourceName res = prefabData.GetPrefabName();			
		if (!res.IsEmpty())
			return true;
				
		m_BuildComposition = ent;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Check the slots if they are still available for building if not, remove preview and building controller on this slot
	void CheckSlots()
	{		
		for (int i = m_aSlotEntities.Count() - 1; i >= 0; i--)
		{
			SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(m_aSlotEntities[i]);
			if (!slotEnt)
			    continue;
				
			//-- Get the controller of the slot we are going to iteract with and return it's component		
			if (slotEnt.IsOccupied())
			{
				IEntity controller = GetControllerEntities().Get(slotEnt);	
				if (!controller)
					continue;
				SCR_CampaignBuildingControllerComponent component = SCR_CampaignBuildingControllerComponent.Cast(controller.FindComponent(SCR_CampaignBuildingControllerComponent));
				
				// Remove current preview with controller
				if (component)
					RemovePreviewEntity(component);
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Add a slot into the array of available slots
	void AddSlot(SCR_SiteSlotEntity slotEnt)
	{
		m_aSlotEntities.Insert(slotEnt);
	}
		
	//------------------------------------------------------------------------------------------------
	// Returns array of slots 
	array<SCR_SiteSlotEntity> GetSlots() 
	{
		return m_aSlotEntities;
	}
		
	//------------------------------------------------------------------------------------------------
	// List of all preview entitites. 
	array<SCR_BasePreviewEntity> GetPreviewEntities()
	{
		return m_aPreviewEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	// List of all controllers used for building compositions
	map<SCR_SiteSlotEntity, IEntity> GetControllerEntities()
	{
		return m_mControllerEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	// Radius in which search for slots
	float GetBuildingRadius()
	{
		return m_fBuildingRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns true, if the search loop for slots (moving vehicle) is running
	bool IsSearchLoopRunning()
	{
		return m_bPreviewUpdateLoop;
	}
	
	//------------------------------------------------------------------------------------------------
	// Set building mode
	void SetBuilding(bool value)
	{
		m_bInBuildingMode = value;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns true, if the player is in building mode
	bool IsBuilding()
	{
		return m_bInBuildingMode;
	}
		
	//------------------------------------------------------------------------------------------------
	//Get the list of all slots in given radius executed ether from spawning previews method or from user action to check if is the user action still valid.
	void GetSlotsNearby(IEntity ent, int radius)
	{
		//-- Clear the array first 
		m_aSlotEntities.Clear();
		//-- Get all slots in given distance
		GetGame().GetWorld().QueryEntitiesBySphere(ent.GetOrigin(), m_fBuildingRadius, FilterEntities, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the distance from center of building area to palyer is bigger then _x
	bool PlayerOutOfBuildingArea(IEntity ent)
	{
		return (vector.Distance(m_vBuildingAreaCenter, ent.GetOrigin()) > m_fBuildingRadius * m_fDeletingMultiplier);
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the truck (supply provider) isn't out of the building radius
	bool TruckOutOfBuildingArea(notnull IEntity ent)
	{
		return (vector.Distance(m_vBuildingAreaCenter, ent.GetOrigin()) > m_fBuildingRadius * m_fDeletingMultiplier);
	}
	
	//------------------------------------------------------------------------------------------------
	// Delete all preview entities
	void RemovePreviewEntities()
	{
		for (int i = m_aPreviewEntities.Count()- 1; i >= 0; i--)
		{			
			SCR_EntityHelper.DeleteEntityAndChildren(GetPreviewEntities()[i]);
		}
		
		m_aPreviewEntities.Clear();
		
		foreach (ImageWidget icon: m_aHUDIcons)
			if (icon)
				icon.RemoveFromHierarchy();
		
		m_aHUDIcons.Clear();
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	// Delete all building controllers
	void RemoveControllers()
	{
		if (!m_mControllerEntities)
			return;
		
		for (int i = m_mControllerEntities.Count()- 1; i >= 0; i--)
		{			
			SCR_EntityHelper.DeleteEntityAndChildren(GetControllerEntities().GetElement(i));
		}
		
		m_mControllerEntities.Clear();
		
		if (m_mControllerEntities.IsEmpty())
			m_mControllerEntities = null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Delete specific preview eintity and it's controler
	// Used on composition change 
	void RemovePreviewEntity(notnull SCR_CampaignBuildingControllerComponent component)
	{
		int itemIndex = GetPreviewEntities().Find(component.GetPreviewEntity());
		
		// Remove composition preview, icon and slot from array
		if (itemIndex != -1)
		{
			GetPreviewEntities().RemoveOrdered(itemIndex);
			m_aHUDIcons[itemIndex].RemoveFromHierarchy();
			m_aHUDIcons.RemoveOrdered(itemIndex);
		}
		
		// Delete preview
		SCR_EntityHelper.DeleteEntityAndChildren(component.GetPreviewEntity());
		// If it's a service composition, delete the controller too.
		if (component.IsServiceComposition())
			SCR_EntityHelper.DeleteEntityAndChildren(component.GetOwner());
	}	
	
	//------------------------------------------------------------------------------------------------
	// Start update loop (moving vehicle)
	void StartUpdatePreviewLoop()
	{
		// start update loop
		m_bPreviewUpdateLoop = true;
		GetGame().GetCallqueue().CallLater(UpdateShownCompositions, UPDATE_TIME_TRUCK, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Stop update loop (moving vehicle)
	void StopUpdatePreviewLoop()
	{
		GetGame().GetCallqueue().Remove(UpdateShownCompositions);
	}
		
	//------------------------------------------------------------------------------------------------
	// remove all EH
	void RemoveHandlers()
	{
		StopUpdatePreviewLoop();
	
		m_bPreviewUpdateLoop = false;
		m_bPlayerInHisSupplyTruck = false;
		m_aPreviewEntities.Clear();
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;
		
		// --- Remove all Event Handlers used for building 
		// Removing slots EH
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core) 
			core.Event_OnEntityExtendedChange.Remove(CheckSlots);
		
		// Removing player EH onDeath
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (characterControllerComponent)
			characterControllerComponent.m_OnPlayerDeath.Remove(OnDeath);
		
		// Removing player access compartment EH
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		if (accessComp)
		{
			accessComp.GetOnCompartmentEntered().Remove(OnSupplyTruckEntered);
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
		}

		// Removing vehicle EH OnSupplyTruckDestroyed
		if (m_VehicleInstance)
		{
			m_DamageManagerVehicle = DamageManagerComponent.Cast(m_VehicleInstance.FindComponent(DamageManagerComponent));
			if (m_DamageManagerVehicle)
			{
				ScriptedHitZone hitZone = ScriptedHitZone.Cast(m_DamageManagerVehicle.GetDefaultHitZone());
				if (hitZone)
					hitZone.GetOnDamageStateChanged().Remove(OnSupplyTruckDestroyed);
			}
		
			SCR_CampaignSuppliesComponent suppliComp = 	GetTruckSuppliesComponent(m_VehicleInstance);
			if (suppliComp)
				suppliComp.m_OnSuppliesTruckDeleted.Remove(OnDelete);		
		}
		
		// Removing base captured EH 
		SCR_CampaignBase.s_OnBaseOwnerChanged.Remove(OnBaseCaptured);
		SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnBaseCaptured);
		
		// Notify gamemode that building mode has ended
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (campaign)
			campaign.OnBuildingInterfaceClosed();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignSuppliesComponent GetTruckSuppliesComponent(notnull IEntity truck)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(truck.FindComponent(SlotManagerComponent));
				
		// It's a supply truck
		if (!slotManager)
			return null;
	
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
		IEntity truckBed;
		SCR_CampaignSuppliesComponent suppliesComp;
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			suppliesComp = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComp)
				return suppliesComp;
		}
		
		return null;
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Draw HUD icons above previews (opacity based on distance)
		BaseWorld world = GetGame().GetWorld();
		vector playerPos = owner.GetOrigin();
		
		for (int i = 0, cnt = m_aPreviewEntities.Count(); i < cnt; i++)
		{
			ImageWidget icon = m_aHUDIcons[i];
			
			if (!icon)
				continue;
			
			vector slotPos = m_aPreviewEntities[i].GetOrigin();
			slotPos[1] = slotPos[1] + 5;
			vector pos = GetGame().GetWorkspace().ProjWorldToScreen(slotPos, world);
			float opacity;
			
			float dist = vector.DistanceSqXZ(playerPos, slotPos);
			
			if (dist >= MAX_HUD_ICON_DISTANCE)
				opacity = 0;
			else
				opacity = Math.Lerp(1, 0, dist / MAX_HUD_ICON_DISTANCE);
			
			if (pos[2] > 0 && opacity != 0)
			{
				icon.SetOpacity(opacity);
				FrameSlot.SetPos(icon, pos[0], pos[1]);
				FrameSlot.SetSize(icon, DEFAULT_HUD_ICON_SIZE * opacity, DEFAULT_HUD_ICON_SIZE * opacity);
			}
			else
				icon.SetOpacity(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Constructor & Destructor
	void SCR_CampaignBuildingComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{			
		m_aPreviewEntities = {};
		m_GameModeCampaignMP = SCR_GameModeCampaignMP.Cast(GetGame().GetGameMode());
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBuildingComponent()
	{
		// Removing slots invoker
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core) 
			return;
		
		core.Event_OnEntityExtendedChange.Remove(CheckSlots);
		
		foreach (Widget w: m_aHUDIcons)
			if (w)
				w.RemoveFromHierarchy();
		
		m_aHUDIcons = null;
	}
};
