[ComponentEditorProps(category: "GameScripted/Editor", description: "Main campaign editor component to handle building mode placing", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingEditorComponentClass : SCR_PlacingEditorComponentClass
{
	[Attribute(params: "et", desc: "List of prefabs that are ignored for clipping check.")]
	protected ref array<ResourceName> m_aClippingCheckIgnoredPrefabs;
	
	bool ContainPrefab(ResourceName res)
	{
		if (m_aClippingCheckIgnoredPrefabs.Contains(res))
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingPlacingEditorComponent : SCR_PlacingEditorComponent
{
	[Attribute("{C7EE4C198B641E21}Sounds/UI/UI_Actions.acp", UIWidgets.ResourceNamePicker, "Sound project file to be used for building", "acp")]
	protected ResourceName m_sSoundFile;

	protected bool m_bCanBeCreated = true;
	protected bool m_bPreviewOutOfArea = false;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_FreeRoamBuildingClientTriggerEntity m_AreaTrigger;
	protected IEntity m_Provider;
	protected ECantBuildNotificationType m_eBlockingReason;

	protected float m_fSafezoneRadius = 10;
	static const float WATER_SURFACE_OFFSET = 0.1; //- 0.4;
	static const float BOUNDING_BOX_FACTOR = 0.25;

	//------------------------------------------------------------------------------------------------
	//! Let the Conflict know player builds (or dissassambled) a service at the base.
	protected void NotifyCampaign(int prefabID, SCR_EditableEntityComponent entity, bool add)
	{
		if (!m_Provider)
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
	//! Return the base which belongs to a provider (if any)
	protected bool GetProviderBase(out SCR_CampaignBase base)
	{
		base = SCR_CampaignBase.Cast(m_Provider);
		if (!base)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Composition preview enters the building area
	protected void PreviewInAreaEvent(IEntity ent)
	{
		SCR_BasePreviewEntity prevEnt = SCR_BasePreviewEntity.Cast(ent);
		if (prevEnt)
		{
			// it's not a root of preview
			if (prevEnt.GetParent())
				return;

			m_bPreviewOutOfArea = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Composition preview left the building area
	protected void PreviewOutOfAreaEvent(IEntity ent)
	{
		SCR_BasePreviewEntity prevEnt = SCR_BasePreviewEntity.Cast(ent);
		if (prevEnt)
		{
			// it's not a root of preview
			if (prevEnt.GetParent())
				return;

			m_bPreviewOutOfArea = true;
			m_bCanBeCreated = false;
			m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the preview of prefab to build is created.
	protected void OnPreviewCreated(SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		SetInitialCanBeCreatedState(previewEnt);
		SetSafezoneRadius(previewEnt);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the initial state of the preview can / can't be created (based on the place where player place the preview into the world.)
	protected void SetInitialCanBeCreatedState(notnull SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		float distance = vector.DistanceSq(m_AreaTrigger.GetOrigin(), previewEnt.GetOrigin());
		if (distance < m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius())
			m_bCanBeCreated = true;

		m_bCanBeCreated = false;
		m_bPreviewOutOfArea = true;
		m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		return;
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
		
		vector position = entity.GetOwner().GetOrigin();
		Rpc(PlaySoundEvent, position, m_sSoundFile);
		PlaySoundEvent(position, m_sSoundFile);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void PlaySoundEvent(vector pos, string soundEvent)
	{
		vector transform[4];
		transform[3] = pos;
		
		if (soundEvent && pos)
			AudioSystem.PlayEvent(soundEvent, SCR_SoundEvent.SOUND_BUILD, transform, new array<string>, new array<float>); 
	}

	//------------------------------------------------------------------------------------------------
	// Add events when the preview of the composition enters or leaves the building area
	protected void SetTriggerEvents()
	{
		if (!m_AreaTrigger)
			return;

		m_AreaTrigger.GetOnEntityEnterTrigger().Insert(PreviewInAreaEvent);
		m_AreaTrigger.GetOnEntityLeaveTrigger().Insert(PreviewOutOfAreaEvent);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveAreaTriggerEvents()
	{
		if (!m_AreaTrigger)
			return;

		m_AreaTrigger.GetOnEntityEnterTrigger().Remove(PreviewInAreaEvent);
		m_AreaTrigger.GetOnEntityLeaveTrigger().Remove(PreviewOutOfAreaEvent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanPlaceEntityServer(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool updatePreview, bool showNotification)
	{
		bool canPlaceManager = true;

		if (IsServiceCapReached(editableEntitySource))
			return false;

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
		if (!m_bPreviewOutOfArea)
			CheckPosition();
		
		SCR_CampaignBuildingPlacingEditorComponentClass componentData = SCR_CampaignBuildingPlacingEditorComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData) 
			return false;

		if (m_bCanBeCreated || componentData.ContainPrefab(GetSelectedPrefab()))
			return true;

		switch (m_eBlockingReason)
		{
			case ECantBuildNotificationType.OUT_OF_AREA: outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE; break;
			case ECantBuildNotificationType.BLOCKED: outNotification = ENotification.EDITOR_PLACING_BLOCKED; break;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		InitVariables();
		SetTriggerEvents();
		SetInitialEvent();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivateServer()
	{
		InitVariables();
		GetOnPlaceEntityServer().Insert(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		super.EOnEditorOpen();
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		super.EOnEditorOpenServer();
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	private void SetInitialEvent()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));
		if (!m_PreviewManager)
			return;

		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	private void InitVariables()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		m_Provider = m_CampaignBuildingComponent.GetProviderEntity();
		m_AreaTrigger = SCR_FreeRoamBuildingClientTriggerEntity.Cast(m_CampaignBuildingComponent.GetTrigger());
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorDeactivate()
	{
		RemoveAreaTriggerEvents();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDeactivateServer()
	{
		GetOnPlaceEntityServer().Remove(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the safe zone radius around the center of the preview in which can't be any entities with simulated physic.
	void SetSafezoneRadius(notnull SCR_EditablePreviewEntity previewEnt)
	{
		vector vectorMin, vectorMax;
		previewEnt.GetPreviewBounds(vectorMin, vectorMax);
		float dist = vector.DistanceXZ(vectorMin, vectorMax);

		m_fSafezoneRadius = dist * BOUNDING_BOX_FACTOR;
	}

	//------------------------------------------------------------------------------------------------
	//! Check the preview position. Is suitable to build the composition here?
	protected bool CheckPosition()
	{
		if (!m_PreviewManager)
			return false;

		BaseWorld world = GetOwner().GetWorld();
		if (!world)
			return false;

		IEntity previewEnt = m_PreviewManager.GetPreviewEntity();
		if (!previewEnt)
			return false;

		vector previewOrigin = previewEnt.GetOrigin();

		// Check clipping with another entity. If can't be placed don't continue.
		if (TraceEntityOnPosition(previewOrigin, world))
			return false;

		// Check if the placing isn't blocked because the origin of the preview is in water.
		// ToDo: Light version of TryGetWaterSurface method which doesn't provide out parameters, just a bool.
		vector outWaterSurfacePoint;
		EWaterSurfaceType outType;
		vector transformWS[4];
		vector obbExtents;
		previewOrigin[1] = previewOrigin[1] + WATER_SURFACE_OFFSET;

		if (ChimeraWorldUtils.TryGetWaterSurface(world, previewOrigin, outWaterSurfacePoint, outType, transformWS, obbExtents))
		{
			if (!m_bCanBeCreated)
				return false;

			m_bCanBeCreated = false;
			return m_bCanBeCreated;
		}

		if (m_bCanBeCreated)
			return true;

		m_bCanBeCreated = true;
		return m_bCanBeCreated;
	}

	//------------------------------------------------------------------------------------------------
	//! Trace at the position of the preview to find any possibly cliping entities.
	protected bool TraceEntityOnPosition(vector position, notnull BaseWorld world)
	{
		TraceSphere sphere = new TraceSphere();
		sphere.Radius = m_fSafezoneRadius;
		sphere.Start = position;
		sphere.LayerMask = EPhysicsLayerPresets.Projectile;
		sphere.Flags = TraceFlags.ENTS | TraceFlags.WORLD;

		float done = world.TracePosition(sphere, null);

		if (done > 0)
		{
			m_bCanBeCreated = true;
			return false;
		}

		m_eBlockingReason = ECantBuildNotificationType.BLOCKED;
		m_bCanBeCreated = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Filter out entities without simulated physic.
	protected bool IsIgnoredBlockingEntity(notnull IEntity ent)
	{
		Physics physics = ent.GetPhysics();
		return (!physics || physics.GetSimulationState() == SimulationState.NONE);
	}
};

enum ECantBuildNotificationType
{
	OUT_OF_AREA = 0,
	BLOCKED = 1,
};
