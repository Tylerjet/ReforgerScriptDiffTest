[ComponentEditorProps(category: "GameScripted/Editor", description: "Main campaign editor component to handle building mode placing", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingEditorComponentClass : SCR_PlacingEditorComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingPlacingEditorComponent : SCR_PlacingEditorComponent
{
	protected bool m_bCanBeCreated = true;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_FreeCampaignBuildingTrigger m_AreaTrigger;
	protected SCR_CampaignBuildingEntityAreaTrigger m_PreviewEntityTrigger;
	protected SCR_BudgetEditorComponent m_MainBudgetManager;
	protected ref ScriptInvoker Event_OnDeactivate = new ScriptInvoker();
	protected ECantBuildNotificationType m_eBlockingReason;
	
	//------------------------------------------------------------------------------------------------
	protected void SetMainBudgetManager(SCR_BudgetEditorComponent mainBudgetManager)
	{
		m_MainBudgetManager = mainBudgetManager;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPreviewEntityTrigger(notnull SCR_CampaignBuildingEntityAreaTrigger trg)
	{
		m_PreviewEntityTrigger = trg;
		if (m_bCanBeCreated)
			AddPreviewAreaTriggerEvents();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckBlocking()
	{
		if (!m_PreviewEntityTrigger)
			return;
		
		m_bCanBeCreated = m_PreviewEntityTrigger.IsNotBlocking();
		if (!m_bCanBeCreated)
			m_eBlockingReason = ECantBuildNotificationType.BLOCKED;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void NotifyCampaign(int prefabID, SCR_EditableEntityComponent entity, bool add)
	{
		if (!m_AreaTrigger)
			return;
		
		SCR_CampaignBase base;
		if (!GetProviderBase(base))
			return;
		
		SCR_GameModeCampaignMP campaignMode = SCR_GameModeCampaignMP.GetInstance();
		if (!campaignMode)
			return;
		
		campaignMode.OnStructureBuilt(base, entity, add);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetProviderBase(out SCR_CampaignBase base)
	{
		base = SCR_CampaignBase.Cast(m_AreaTrigger.GetParent());
		if (!base)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PreviewInAreaEvent(IEntity ent)
	{
		SCR_BasePreviewEntity prevEnt = SCR_BasePreviewEntity.Cast(ent);
		if (prevEnt)
		{
			AddPreviewAreaTriggerEvents();
			m_bCanBeCreated = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PreviewOutOfAreaEvent(IEntity ent)
	{
		SCR_BasePreviewEntity prevEnt = SCR_BasePreviewEntity.Cast(ent);
		if (prevEnt)
		{
			RemovePreviewAreaTriggerEvents();
			m_bCanBeCreated = false;
			m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetInitialCanBeCreatedState(SCR_EditablePreviewEntity previewEnt)
	{
		float distance = vector.DistanceSq(m_AreaTrigger.GetOrigin(), previewEnt.GetOrigin());
		if (distance > m_AreaTrigger.GetSphereRadius())
		{
			m_bCanBeCreated = false;
			m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
			return;
		}
		
		m_bCanBeCreated = true;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBeforeEntityCreatedServer(ResourceName prefab)
	{
		// Get SCR_CampaignBuildingManagerComponent (on Game mode) here and set temporary owner.
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent || !m_CampaignBuildingComponent)
			return;
		
		buildingManagerComponent.SetTemporaryProvider(m_CampaignBuildingComponent.GetProviderEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEntityCreatedServer(array<SCR_EditableEntityComponent> entities)
	{
		if (!m_CampaignBuildingComponent)
			return;
		
		int count = entities.Count();
		for (int i = 0; i < count; i++)
		{
			IEntity entityOwner = entities[i].GetOwnerScripted();
			if (!entityOwner)
				continue;
			
			SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entityOwner.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (!compositionComponent)
				continue;
			
			SetProvider(compositionComponent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the provider entity to composition. Provider is the entity to which a composition belong to. For example a base, supply truck...
	protected void SetProvider(notnull SCR_CampaignBuildingCompositionComponent compositionComponent)
	{
		IEntity provider = m_CampaignBuildingComponent.GetProviderEntity();
		if (!provider)
			return;

		compositionComponent.SetProviderEntity(provider);

		// Don't set up this hook if the provider is a base. We don't want to change the ownership here
		SCR_CampaignBase base = SCR_CampaignBase.Cast(provider);
		if (base)
			return;
		
		SCR_EditorModeEntity ent = SCR_EditorModeEntity.Cast(GetOwner());
		compositionComponent.SetClearProviderEvent(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlaceEntityServer(int prefabID, SCR_EditableEntityComponent entity)
	{
		NotifyCampaign(prefabID, entity, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Add events when the preview of the composition enters or leaves the building area
	protected void SetTriggerEvents()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;
		
		m_AreaTrigger = SCR_FreeCampaignBuildingTrigger.Cast(m_CampaignBuildingComponent.GetTrigger());
		if (!m_AreaTrigger)
			return;
		
		m_AreaTrigger.GetOnEntityEnterTrigger().Insert(PreviewInAreaEvent);
 		m_AreaTrigger.GetOnEntityLeaveTrigger().Insert(PreviewOutOfAreaEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	// add events when the entity enter or leave a trigger attached to a preview of the entity 
	protected void AddPreviewAreaTriggerEvents()
	{
		if (m_PreviewEntityTrigger)
		{
			m_PreviewEntityTrigger.GetOnEntityEnterTrigger().Insert(CheckBlocking);
			m_PreviewEntityTrigger.GetOnEntityLeaveTrigger().Insert(CheckBlocking);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveAreaTriggerEvents()
	{
		if (m_AreaTrigger)
		{
			m_AreaTrigger.GetOnEntityEnterTrigger().Remove(PreviewInAreaEvent);
			m_AreaTrigger.GetOnEntityLeaveTrigger().Remove(PreviewOutOfAreaEvent);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemovePreviewAreaTriggerEvents()
	{
		if (m_PreviewEntityTrigger)
		{
			m_PreviewEntityTrigger.GetOnEntityEnterTrigger().Remove(CheckBlocking);
			m_PreviewEntityTrigger.GetOnEntityLeaveTrigger().Remove(CheckBlocking);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override protected bool CanPlaceEntityServer(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool updatePreview, bool showNotification)
	{
		bool canPlaceManager = true;
		
		if (IsServiceCapReached(editableEntitySource))
			return false;
		
		if (m_MainBudgetManager)
		{
			canPlaceManager = m_MainBudgetManager.CanPlaceEntitySource(editableEntitySource, blockingBudget, HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER), false, false);
		}
		
		return canPlaceManager && super.CanPlaceEntityServer(editableEntitySource, blockingBudget, updatePreview, showNotification);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsServiceCapReached(IEntityComponentSource editableEntitySource)
	{
		SCR_CampaignBase base;
		if (!GetProviderBase(base))
			return false;
		
		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (!editableUIInfo)
			return false;
		
		array<EEditableEntityLabel> entityLabels = {};
		array<SCR_CampaignServiceComponent> baseServices = {};
		editableUIInfo.GetEntityLabels(entityLabels);
		int count = base.GetAllBaseServices(baseServices);
		
		for (int i = 0; i < count; i++)
		{
			if (entityLabels.Contains(baseServices[i].GetLabel()))
			{
				SCR_NotificationsComponent.SendToPlayer(GetManager().GetPlayerID(), ENotification.EDITOR_PLACING_NO_MORE_INSTANCES);
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanCreateEntity(out ENotification outNotification = -1)
	{
		if (m_bCanBeCreated)
			return true;
		
		switch (m_eBlockingReason)
		{
			case ECantBuildNotificationType.OUT_OF_AREA: outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE; break;
			case ECantBuildNotificationType.BLOCKED: outNotification = ENotification.EDITOR_PLACING_BLOCKED; break;
		}
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDeactivate()
	{
		return Event_OnDeactivate;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		m_MainBudgetManager = SCR_BudgetEditorComponent.Cast(FindEditorComponent(SCR_BudgetEditorComponent, true, false));
		SetTriggerEvents();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivateServer()
	{
		m_MainBudgetManager = SCR_BudgetEditorComponent.Cast(GetManager().FindComponent(SCR_BudgetEditorComponent));
		SetTriggerEvents();
		
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));
		if (!m_PreviewManager)
			return;
		
		m_PreviewManager.GetOnPreviewCreate().Insert(SetInitialCanBeCreatedState);		
		
		GetOnPlaceEntityServer().Insert(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorDeactivate()
	{
		RemoveAreaTriggerEvents();
		RemovePreviewAreaTriggerEvents();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDeactivateServer()
	{	
		if (m_PreviewManager)
			m_PreviewManager.GetOnPreviewCreate().Remove(SetInitialCanBeCreatedState);	
		
		GetOnPlaceEntityServer().Remove(OnPlaceEntityServer);
		
		RemoveAreaTriggerEvents();
		RemovePreviewAreaTriggerEvents();
		
		Event_OnDeactivate.Invoke();
	}
};

enum ECantBuildNotificationType
{
	OUT_OF_AREA = 0,
	BLOCKED = 1,
};