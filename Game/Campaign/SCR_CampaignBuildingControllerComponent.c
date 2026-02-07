[EntityEditorProps(category: "GameScripted/Campaign", description: "Component on preview controller. Contains data about preview, handle it's rotation", color: "0 0 255 255")]
class SCR_CampaignBuildingControllerComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingControllerComponent : ScriptComponent
{
	//! Material used for preview when the composition can be build
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Preview material can be build", "emat")]
	private ResourceName m_CanBuildMaterial;
	
	//! Material used for preview when the composition can't be build
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Preview material can't be build", "emat")]
	private ResourceName m_CannotBuildMaterial;
	
	//! Prefab of the trigger used to detect a obstruction / player presence in building area.
	[Attribute("", UIWidgets.ResourceNamePicker, "Building trigger", "et")]
	protected ResourceName m_BuildingTrigger;
	
	private SCR_CampaignBase m_Base;	
	private SCR_CampaignSuppliesComponent m_SuppliesComponent;
	private SCR_CampaignBuildingComponent m_BuildingComponent;
	private SCR_CampaignSlotComposition m_SlotData;
	private SCR_CampaignBuildingClientTrigger m_Trigger;
	private SCR_BasePreviewEntity m_PreviewEntity;
	private InputManager m_InputManager;
	private SCR_SiteSlotEntity m_UsedSlot;
	private vector m_vNewAngle;
	private IEntity m_SuppliesProvider;
	private float m_fCostMultiplier = 1;
	private float m_fRefundMultiplier = 1;
	
	static const int ROTATION_STEP_BASE = 1;
	static const int ROTATION_KEYBOARD_MULTIPLIER = 8;
	static const string BUILDING_CONTEXT = "BuildingContext";
	// Radiuses for all slots has been reduce as we spawn only the small compositions in all types of slots.
	static const int SLOT_SMALL_RADIUS = 5;
	static const int SLOT_MEDIUM_RADIUS = 5;
	static const int SLOT_LARGE_RADIUS = 5;
	static const int SLOT_ROAD_SMALL_RADIUS = 4;
	static const int SLOT_ROAD_MEDIUM_RADIUS = 7;
	static const int SLOT_ROAD_LARGE_RADIUS = 9;
		
	private BaseInteractionHandlerComponent m_InteractionMenu;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner) 
	{		
		// Get intraction menu
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			m_InteractionMenu = BaseInteractionHandlerComponent.Cast(pc.FindComponent(BaseInteractionHandlerComponent));
		
		// Get buidling component
		IEntity playerEnt = SCR_PlayerController.GetLocalControlledEntity();
		if (playerEnt)
			m_BuildingComponent = SCR_CampaignBuildingComponent.Cast(playerEnt.FindComponent(SCR_CampaignBuildingComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnTrigger()
	{
		//Origin of the slot has to be used, because controller is spawned with an offset.
		if (!m_UsedSlot)
			return;
		
		Resource resource = Resource.Load(m_BuildingTrigger);
		if (!resource.IsValid())
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_UsedSlot.GetOrigin(); 
			
		m_Trigger = SCR_CampaignBuildingClientTrigger.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params));
		if (!m_Trigger)
			return;
		
		m_Trigger.SetBuildingController(this);
		m_Trigger.SetOrigin(GetOwner().GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarker()
	{
		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_MapDescriptorComponent));
		
		if (descr)
		{
			MapItem item = descr.Item();
			
			if (!item)
				return; 
			
			MapDescriptorProps props = item.GetProps();
			item.SetImageDef(GetMarkerImage());
			props.SetFrontColor(Color.White);
			props.SetTextVisible(false);
			props.SetIconSize(32, 0.5, 0.5);
			props.Activate(true);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	bool HasAvailableResources()
	{
		if (m_SuppliesComponent && m_SuppliesComponent.GetSupplies() >= GetPrice())
		{
			return true;
		}	
		
		if (m_Base && m_Base.GetSupplies() >= GetPrice())
		{
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Colorize composition
	void SetCompositionColor()
	{
		if (!m_PreviewEntity || !GetTrigger())
			return;
				
		// has no resources, can't build
		if (!HasAvailableResources())
		{
			SCR_Global.SetMaterial(m_PreviewEntity.GetChildren(), m_CannotBuildMaterial);
			return;
		}
				
		// Build action was executed but the area is blocked. (vehicle, AI...)
		if (GetTrigger().IsBlocked() && GetTrigger().IsToBeBuilt())
		{
			SCR_Global.SetMaterial(m_PreviewEntity.GetChildren(), m_CannotBuildMaterial);
			return;
		}

		SCR_Global.SetMaterial(m_PreviewEntity.GetChildren(), m_CanBuildMaterial);
	}
		
	//------------------------------------------------------------------------------------------------
	void ActivateActionListeners()
	{
 		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("BuildingPreviewRotationUp", EActionTrigger.PRESSED, PreviewRotationUp);
		m_InputManager.AddActionListener("BuildingPreviewRotationDown", EActionTrigger.PRESSED, PreviewRotationDown);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateActionListeners()
	{
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.RemoveActionListener("BuildingPreviewRotationUp", EActionTrigger.PRESSED, PreviewRotationUp);
		m_InputManager.RemoveActionListener("BuildingPreviewRotationDown", EActionTrigger.PRESSED, PreviewRotationDown);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStartAngle(vector startAngle)
	{
		m_vNewAngle = startAngle;
	}
	//------------------------------------------------------------------------------------------------
	void SetEntity(SCR_BasePreviewEntity ent)
	{
		m_PreviewEntity = ent;
	}
	
	//------------------------------------------------------------------------------------------------
 	void SetSuppliesProvider(notnull IEntity suppliesProvider)
	{		
		m_SuppliesProvider = suppliesProvider;
		
		// It's a base
		m_Base = SCR_CampaignBase.Cast(suppliesProvider);
		if (m_Base)
		{
			SCR_CampaignServiceComponent supplyDepotService = m_Base.GetBaseService(ECampaignServicePointType.SUPPLY_DEPOT);
			
			if (!supplyDepotService)
				return;
			
			IEntity supplyDepot = supplyDepotService.GetOwner();
			
			if (!supplyDepot)
				return;
			
			SCR_CampaignSuppliesComponent supp = SCR_CampaignSuppliesComponent.Cast(supplyDepot.FindComponent(SCR_CampaignSuppliesComponent));
		
			if (supp)
				supp.m_OnSuppliesChanged.Insert(SetCompositionColor);
			
			SetCompositionColor();
			return;
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(suppliesProvider.FindComponent(SlotManagerComponent));
		
		// It's a supply truck
		if (slotManager)
		{
			array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
			slotManager.GetSlotInfos(slots);
			
			foreach (EntitySlotInfo slot: slots)
			{
				if (!slot)
					continue;
				
				IEntity truckBed = slot.GetAttachedEntity();
				
				if (!truckBed)
					continue;
				
				SCR_CampaignSuppliesComponent comp = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
				
				if (comp)
				{
					m_SuppliesComponent = comp;
					m_SuppliesComponent.m_OnSuppliesChanged.Insert(SetCompositionColor);
				}
				
				SetCompositionColor();
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetData(notnull SCR_CampaignSlotComposition slotData)
	{
		m_SlotData = slotData;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetUsedSlot(notnull SCR_SiteSlotEntity usedSlot)
	{
		m_UsedSlot = usedSlot;
		SetTriggerSize(usedSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTriggerSize(notnull SCR_SiteSlotEntity usedSlot)
	{
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager || !m_Trigger)
			return;
		
		ResourceName resName = usedSlot.GetPrefabData().GetPrefabName();
		for (int i = 0; i <= SCR_ESlotTypesEnum.CheckpointLarge; i++ )
		{
			if (resName == factionManager.GetSlotsResource(i))
			{
				switch (i)
				{
					case SCR_ESlotTypesEnum.FlatSmall: {m_Trigger.SetSphereRadius(SLOT_SMALL_RADIUS); break;};
					case SCR_ESlotTypesEnum.FlatMedium: {m_Trigger.SetSphereRadius(SLOT_MEDIUM_RADIUS); break;};
					case SCR_ESlotTypesEnum.FlatLarge: {m_Trigger.SetSphereRadius(SLOT_LARGE_RADIUS); break;};
					case SCR_ESlotTypesEnum.CheckpointSmall: {m_Trigger.SetSphereRadius(SLOT_ROAD_SMALL_RADIUS); break;};
					case SCR_ESlotTypesEnum.CheckpointMedium: {m_Trigger.SetSphereRadius(SLOT_ROAD_MEDIUM_RADIUS); break;};
					case SCR_ESlotTypesEnum.CheckpointLarge: {m_Trigger.SetSphereRadius(SLOT_ROAD_LARGE_RADIUS); break;};
				}
			}	
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCostMultiplier(float costMultiplier)
	{
		m_fCostMultiplier = costMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRefundMultiplier(float refundCostMultiplier)
	{
		m_fRefundMultiplier = refundCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignSlotComposition GetUsedData()
	{
		return m_SlotData;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPrice()
	{
		return m_SlotData.GetPrice() * m_fCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRefundValue()
	{
		return m_SlotData.GetPrice() * m_SlotData.GetRefundPercentage() * m_fRefundMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMarkerImage()
	{
		return m_SlotData.GetMarkerImage();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetSuppliesProvider()
	{
		return m_SuppliesProvider; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBeRotated()
	{
		return m_SlotData.CanBeRotated();
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetResource()
	{
		return m_SlotData.GetResourceName();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCompositionName()
	{
		return m_SlotData.GetCompositionName();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsServiceComposition()
	{
		return m_SlotData.IsServiceComposition();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SiteSlotEntity GetUsedSlot()
	{
		return m_UsedSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BasePreviewEntity GetPreviewEntity()
	{
		return m_PreviewEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetAngle()
	{
		return m_vNewAngle;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBuildingClientTrigger GetTrigger()
	{
		return m_Trigger;
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateController()
	{
		GetOwner().SetFlags(EntityFlags.ACTIVE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateController()
	{
		GetOwner().ClearFlags(EntityFlags.ACTIVE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rotate with composition preview
	void RotatePreview()
	{		
		// Don't continue in case player is interacting with action menu
		if (m_InteractionMenu && m_InteractionMenu.IsInteractionAvailable())
			return;

		if (m_InputManager && !m_InputManager.IsContextActive(BUILDING_CONTEXT))
			m_InputManager.ActivateContext(BUILDING_CONTEXT,100);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PreviewRotationUp()
	{		
		int rotation = ROTATION_STEP_BASE;
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			rotation = rotation * ROTATION_KEYBOARD_MULTIPLIER;
		
		vector currentAngle = m_PreviewEntity.GetAngles();
		m_vNewAngle = Vector(currentAngle[0],currentAngle[1] + rotation,currentAngle[2]);
		m_PreviewEntity.SetAngles(m_vNewAngle);
		if (m_BuildingComponent)
			m_BuildingComponent.UpdateCompositionOrientation(m_PreviewEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PreviewRotationDown()
	{		
		int rotation = ROTATION_STEP_BASE;
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			rotation = rotation * ROTATION_KEYBOARD_MULTIPLIER;
		
		vector currentAngle = m_PreviewEntity.GetAngles();
		m_vNewAngle = Vector(currentAngle[0],currentAngle[1] - rotation,currentAngle[2]);
		m_PreviewEntity.SetAngles(m_vNewAngle);
		if (m_BuildingComponent)
			m_BuildingComponent.UpdateCompositionOrientation(m_PreviewEntity);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBuildingControllerComponent()
	{
		SCR_EntityHelper.DeleteEntityAndChildren(m_Trigger);
		
		// Remove EH to change color
		if (m_Base)
		{
			SCR_CampaignServiceComponent supplyDepotService = m_Base.GetBaseService(ECampaignServicePointType.SUPPLY_DEPOT);
			
			if (supplyDepotService)
			{
				IEntity supplyDepot = supplyDepotService.GetOwner();
			
				if (supplyDepot)
				{
					SCR_CampaignSuppliesComponent supp = SCR_CampaignSuppliesComponent.Cast(supplyDepot.FindComponent(SCR_CampaignSuppliesComponent));
					
					if (supp)
						supp.m_OnSuppliesChanged.Remove(SetCompositionColor);
				}
			}
			
			return;
		}
		
		if (m_SuppliesComponent)
			m_SuppliesComponent.m_OnSuppliesChanged.Remove(SetCompositionColor);	
	}
};