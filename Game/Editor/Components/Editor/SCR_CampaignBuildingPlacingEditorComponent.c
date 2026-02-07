[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict editor component to handle building mode placing", icon: "WBData/ComponentEditorProps/componentEditor.png")]
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
	[Attribute("{C7EE4C198B641E21}Sounds/Editor/BaseBuilding/Editor_BaseBuilding.acp", UIWidgets.ResourceNamePicker, "Sound project file to be used for building", "acp")]
	protected ResourceName m_sSoundFile;

	protected bool m_bCanBeCreated = true;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_FreeRoamBuildingClientTriggerEntity m_AreaTrigger;
	protected IEntity m_Provider;
	protected ECantBuildNotificationType m_eBlockingReason;
	protected SCR_EditablePreviewEntity m_PreviewEnt;
	protected ref array<ref Tuple2<SCR_BasePreviewEntity, float>> m_CompositionEntities = {};

	protected float m_fSafezoneRadius = 10;
	//Adding this value to a sea level as the composition preview, even above sea doesn't have it's Y value exactly a zero.
	protected static const float SEA_LEVEL_OFFSET = 0.01;
	protected static const float BOUNDING_BOX_FACTOR = 0.3;
	
	//------------------------------------------------------------------------------------------------
	//! Return the base which belongs to a provider (if any)
	protected bool GetProviderBase(out SCR_CampaignMilitaryBaseComponent base)
	{
		base = SCR_CampaignMilitaryBaseComponent.Cast(m_Provider.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!base)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the preview of prefab to build is created.
	protected void OnPreviewCreated(SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		m_PreviewEnt = previewEnt;
		SetInitialCanBeCreatedState(previewEnt);
		SetSafezoneRadius(previewEnt);
		GetAllEntitiesToEvaluate(previewEnt);
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
			
			SetProviderAndBuilder(compositionComponent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set the provider and builder entity to composition. Provider is the entity to which a composition belong to. For example a base, supply truck... Builder is a player who build it.
	protected void SetProviderAndBuilder(notnull SCR_CampaignBuildingCompositionComponent compositionComponent)
	{
		IEntity provider = m_CampaignBuildingComponent.GetProviderEntity();
		if (!provider)
			return;
		
		SCR_EditorManagerEntity managerEnt = GetManager();
			if (!managerEnt)
				return;
			
		int id = managerEnt.GetPlayerID();

		compositionComponent.SetBuilderId(id);
		compositionComponent.SetProviderEntity(provider);

		// Don't set up this hook if the provider is a base. We don't want to change the ownership here
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(provider.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (base)
			return;

		SCR_EditorModeEntity ent = SCR_EditorModeEntity.Cast(GetOwner());
		compositionComponent.SetClearProviderEvent(ent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlaceEntityServer(int prefabID, SCR_EditableEntityComponent entity)
	{
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
		SCR_CampaignMilitaryBaseComponent base;
		if (!GetProviderBase(base))
			return false;

		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (!editableUIInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		array<SCR_ServicePointComponent> baseServices = {};
		editableUIInfo.GetEntityLabels(entityLabels);
		int count = base.GetServices(baseServices);

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
	// Check if the preview is outisde of the building radius. if preview doesn't exist return false - preview doesn't exist on server and the same CanCreateEntity is used on server too.
	bool IsPreviewOutOfRange()
	{
		if (!m_AreaTrigger || !m_PreviewEnt)
			return false;

		return (vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), m_PreviewEnt.GetOrigin()) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanCreateEntity(out ENotification outNotification = -1)
	{
		if (IsPreviewOutOfRange())
			return false;

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
	override protected void EOnEditorDeactivateServer()
	{
		GetOnPlaceEntityServer().Remove(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the safe zone radius around the center of the preview in which can't be any entities with simulated physic.
	protected void SetSafezoneRadius(notnull SCR_EditablePreviewEntity previewEnt)
	{
		vector vectorMin, vectorMax;
		previewEnt.GetPreviewBounds(vectorMin, vectorMax);
		float dist = vector.DistanceXZ(vectorMin, vectorMax);
		m_fSafezoneRadius = dist * BOUNDING_BOX_FACTOR;
	}

	//------------------------------------------------------------------------------------------------
	// Make an array of all entities to evaluate
	protected void GetAllEntitiesToEvaluate(notnull SCR_EditablePreviewEntity rootEnt)
	{
		m_CompositionEntities.Clear();

		array<SCR_BasePreviewEntity> previewEntities = rootEnt.GetPreviewChildren();
		if (previewEntities.IsEmpty())
			return;

		foreach (SCR_BasePreviewEntity ent : previewEntities)
		{
			array<SCR_BasePreviewEntity> previewEntities1 = ent.GetPreviewChildren();
			if (!previewEntities1)
			{

				// one entity composition. it's root has to go to the list.
				float protectionRadius = GetEntityProtectionRadius(ent);
				if (protectionRadius > 0)
				{
					if (ent.GetOrigin()[1] > 0)
						continue;

					m_CompositionEntities.Insert(new Tuple2<SCR_BasePreviewEntity, float>(ent, protectionRadius));
				}

				return;
			}

			foreach (SCR_BasePreviewEntity ent1 : previewEntities1)
			{
				float protectionRadius = GetEntityProtectionRadius(ent1);
				if (protectionRadius > 0)
				{
					if (ent.GetOrigin()[1] > 0)
						continue;

					m_CompositionEntities.Insert(new Tuple2<SCR_BasePreviewEntity, float>(ent1, protectionRadius));
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Check the preview position. Is suitable to build the composition here?
	protected bool CheckPosition()
	{
		m_bCanBeCreated = true;

		foreach (Tuple2<SCR_BasePreviewEntity, float> compositionEntity : m_CompositionEntities)
		{
			if (compositionEntity.param1 && !CheckEntityPosition(compositionEntity.param1.GetOrigin(), compositionEntity.param2))
			{
				m_bCanBeCreated = false;
				break;
			}
		}

		return m_bCanBeCreated;
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate a sphere radius about the entity which will be tested for obstruction
	protected float GetEntityProtectionRadius(notnull SCR_BasePreviewEntity ent)
	{
		vector vectorMin, vectorMax;
		ent.GetBounds(vectorMin, vectorMax);
		return vector.DistanceXZ(vectorMin, vectorMax) * BOUNDING_BOX_FACTOR;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckEntityPosition(vector pos, float safeZoneRadius)
	{
		BaseWorld world = GetOwner().GetWorld();
		if (!world)
			return false;

		// First do the sea level check as it is cheep and don't need to continue with trace if the composition is in the sea.
		if (pos[1] < world.GetOceanBaseHeight() + SEA_LEVEL_OFFSET)
			return false;

		// Check clipping with another entity. If can't be placed don't continue.
		if (TraceEntityOnPosition(pos, world, safeZoneRadius))
			return false;

		// Check if the placing isn't blocked because the origin of the preview is in water.
		// ToDo: Light version of TryGetWaterSurface method which doesn't provide out parameters, just a bool.
		vector outWaterSurfacePoint;
		EWaterSurfaceType outType;
		vector transformWS[4];
		vector obbExtents;

		if (ChimeraWorldUtils.TryGetWaterSurface(world, pos, outWaterSurfacePoint, outType, transformWS, obbExtents))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Trace at the position of the preview to find any possibly cliping entities.
	protected bool TraceEntityOnPosition(vector position, notnull BaseWorld world, float safeZoneRadius)
	{
		TraceSphere sphere = new TraceSphere();
		sphere.Radius = safeZoneRadius;
		sphere.Start = position;
		sphere.LayerMask = EPhysicsLayerPresets.Projectile;
		sphere.Flags = TraceFlags.ENTS | TraceFlags.WORLD;

		float done = world.TracePosition(sphere, null);
		if (done > 0)
			return false;

		m_eBlockingReason = ECantBuildNotificationType.BLOCKED;
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
