[ComponentEditorProps(category: "GameScripted/Editor", description: "Content browser component. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PlacingEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Placing", uiwidget: UIWidgets.Flags, desc: "System type of the entity.", enums: ParamEnumArray.FromEnum(EEditorPlacingFlags))]
	protected EEditorPlacingFlags m_AllowedPlacingFlags;
	
	[Attribute(category: "Placing", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Editable entity used for testing. Should be a sphere 1 m in diameter.")]
	protected ResourceName m_TestPrefab;
	
	[Attribute(category: "Placing")]
	protected ref array<ref SCR_PlaceableEntitiesRegistry> m_Registries;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceStart;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceConfirm;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceCancel;
	
	protected int m_iPrefabCount;
	protected ref array<int> m_aIndexes = {};
	
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceStart()
	{
		return m_EffectsPlaceStart;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceConfirm()
	{
		return m_EffectsPlaceConfirm;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceCancel()
	{
		return m_EffectsPlaceCancel;
	}
	
	/*
	Get registered prefab with given index.
	\param index Prefab index from the registry
	\return Prefab path (empty when the index is invalid)
	*/
	ResourceName GetPrefab(int index)
	{
		for (int i = m_aIndexes.Count() - 1; i >= 0; i--)
		{
			int registryIndex = m_aIndexes[i];
			if (index >= registryIndex)
				return m_Registries[i].GetPrefabs()[index - registryIndex];
		}
		return ResourceName.Empty;
	}
	/*
	Get index of registered prefab
	\param prefab Prefab path
	\return Prefab index (-1 when the prefab is not found)
	*/
	int GetPrefabID(ResourceName prefab)
	{
		foreach (int i, SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			int index = registry.GetPrefabs().Find(prefab);
			if (index != -1)
				return index + m_aIndexes[i];
		}
		return -1;
	}
	/*
	Get number of prefabs in the registry.
	\return Number of prefabs
	*/
	int CountPrefabs()
	{
		return m_iPrefabCount;
	}
	/*
	Get registered prefabs.
	\param[out] outPrefabs Array to be filled with prefabs
	\return Number of prefabs
	*/
	int GetPrefabs(out notnull array<ResourceName> outPrefabs, bool onlyExposed = false)
	{
		foreach (SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			outPrefabs.InsertAll(registry.GetPrefabs(onlyExposed));
		}
		return outPrefabs.Count();
	}
	/*
	Check if given placing flag is allowed to be changed.
	\return True when the flag can be changed
	*/
	bool HasPlacingFlag(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_AllowedPlacingFlags, flag);
	}
	/*
	Get prefab used for testing.
	\return Prefab
	*/
	ResourceName GetTestPrefab()
	{
		return m_TestPrefab;
	}
	
	void SCR_PlacingEditorComponentClass(BaseContainer prefab)
	{
		foreach (SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			m_aIndexes.Insert(m_iPrefabCount);
			m_iPrefabCount += registry.GetPrefabs().Count();
		}
	}
};

/** @ingroup Editor_Components
*/
class SCR_PlacingEditorComponent : SCR_BaseEditorComponent
{	
	private ResourceName m_SelectedPrefab;
	private SCR_StatesEditorComponent m_StatesManager;
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	private SCR_BudgetEditorComponent m_BudgetManager;
	private SCR_PlacingEditorComponentClass m_PrefabData;
	private SCR_SiteSlotEntity m_Slot;
	private int m_iEntityIndex;
	private ref map<int, IEntity> m_WaitingPreviews = new map<int, IEntity>;
	private vector m_vFixedPosition;
	private float m_fPreviewAnimationProgress = -1;
	private EEditorPlacingFlags m_PlacingFlags;
	private EEditorPlacingFlags m_CompatiblePlacingFlags;
	private ref set<SCR_EditableEntityComponent> m_Recipients;
	private ref SCR_EditorPreviewParams m_InstantPlacingParam;
	
	private ref ScriptInvoker Event_OnSelectedPrefabChange = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlacingPlayerChange = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlacingFlagsChange = new ScriptInvoker;
	
	private ref ScriptInvoker Event_OnRequestEntity = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlaceEntity = new ScriptInvoker;
	
	/*!
	Create entity exactly where the preview entity is.
	The entity is created from a prefab defined by SetSelectedPrefab().
	\param unselectPrefab True to unselect prefab after placing the entity
	\param canBePlayer True if the entity should be spawned as player
	\return True if the request was sent
	*/
	bool CreateEntity(bool unselectPrefab = true, bool canBePlayer = false)
	{
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData || !m_SelectedPrefab) return false;
		
		if (!m_PreviewManager.IsChange() && !m_InstantPlacingParam)
		{
			SendNotification(ENotification.EDITOR_TRANSFORMING_INCORRECT_POSITION);
			SetSelectedPrefab(ResourceName.Empty, true);
			return false;
		}
		
		int prefabID = prefabData.GetPrefabID(m_SelectedPrefab);
		if (prefabID == -1)
		{
			Print(string.Format("Cannot place prefab @\"%1\", it's not registered in placeable entities!", m_SelectedPrefab.GetPath()), LogLevel.WARNING);
			SetSelectedPrefab(ResourceName.Empty, true);
			return false;
		}
		
		//--- If the entity can be spawned as player, get player ID
		int playerID = 0;
		if (canBePlayer || HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
		{
			Resource prefabResource = Resource.Load(m_SelectedPrefab);
			IEntityComponentSource component = SCR_BaseContainerTools.FindComponentSource(prefabResource, SCR_EditableEntityComponent);
			if (component && SCR_EditableEntityComponentClass.GetEntityType(component) != EEditableEntityType.CHARACTER)
			{
				SendNotification(ENotification.EDITOR_PLACING_CANNOT_AS_PLAYER);
				SetSelectedPrefab(ResourceName.Empty, true);
				return false;
			}
			playerID = GetManager().GetPlayerID();
		}
		
		//--- Create packet
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(GetInstance(SCR_LayersEditorComponent));
		SCR_EditorPreviewParams params;
		if (m_InstantPlacingParam)
			params = m_InstantPlacingParam;
		else
			params = SCR_EditorPreviewParams.CreateParamsFromPreview(m_PreviewManager, layersManager.GetCurrentLayer(), true);
		
		if (!params)
			return false;
		
		//--- Save placing flags
		params.m_PlacingFlags = m_PlacingFlags;
		
		//--- Copy the preview, it will remain visible until callback from server is received
		int simulatedDelay = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_NETWORK_DELAY) * 100;
		m_iEntityIndex++;
		if (IsProxy() || simulatedDelay > 0)
			m_WaitingPreviews.Insert(m_iEntityIndex, m_PreviewManager.CreateWaitingPreview());
		
		array<RplId> recipientIds;
		if (m_Recipients)
		{
			recipientIds = {};
			foreach (SCR_EditableEntityComponent entity: m_Recipients)
			{
				RplId id = Replication.FindId(entity);
				if (id.IsValid())
					recipientIds.Insert(id);
			}
		}
		
		//--- Stop placing (only after packet was created)
		if (unselectPrefab || m_InstantPlacingParam)
			SetSelectedPrefab(ResourceName.Empty, true);
		
		//--- Send request to server
		m_StatesManager.SetIsWaiting(true);
		Event_OnRequestEntity.Invoke(prefabID, params.m_vTransform, null);
		
		if (simulatedDelay > 0 && !Replication.IsRunning())
			GetGame().GetCallqueue().CallLater(CreateEntityServer, simulatedDelay, false, params, prefabID, playerID, m_iEntityIndex, !unselectPrefab, recipientIds);
		else
			Rpc(CreateEntityServer, params, prefabID, playerID, m_iEntityIndex, !unselectPrefab, recipientIds);
		
		//--- ToDo: Fail the request if server didn't respond in given time
		//GetGame().GetCallqueue().CallLater(CreateEntityOwner, 10000, false, prefabID, -1);
		
		m_Slot = null;
		m_InstantPlacingParam = null;
		
		return true;
	}
	/*!
	Create entity with custom params.
	\param prefab Entity prefab
	\param param Preview param
	\param unselectPrefab True to unselect prefab after placing the entity
	\param canBePlayer True if the entity should be spawned as player
	\param recipients Array of entities for whom new entities will be created (e.g., waypoints for groups)
	\return True if the request was sent
	*/
	bool CreateEntity(ResourceName prefab, SCR_EditorPreviewParams param, bool unselectPrefab = true, bool canBePlayer = false, set<SCR_EditableEntityComponent> recipients = null)
	{
		m_SelectedPrefab = prefab;
		m_Recipients = recipients;
		SetInstantPlacing(param);
		return CreateEntity(unselectPrefab, canBePlayer);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CreateEntityServer(SCR_EditorPreviewParams params, RplId prefabID, int playerID, int entityIndex, bool isQueue, array<RplId> recipientIds)
	{
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (RplSession.Mode() == RplMode.Client || !prefabData) return;
		
		array<RplId> entityIds = {};
		
		if (!params.Deserialize())
		{
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex);
			return;
		}
		
		//--- Get prefab
		ResourceName prefab = prefabData.GetPrefab(prefabID);
		if (prefab.IsEmpty())
		{
			Print("Cannot create entity, prefab not defined!", LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex);
			return;
		}
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource || !prefabResource.IsValid())
		{
			Print(string.Format("Cannot create entity, error when loading prefab '%1'!", prefab), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex);
			return;
		}
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
		if (!editableEntitySource)
		{
			Print(string.Format("Cannot create entity, prefab '%1' does not contain SCR_EditableEntityComponent!", prefab), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex);
			return;
		}
		EEditableEntityBudget blockingBudget;
		if (!CanPlaceEntityServer(editableEntitySource, blockingBudget))
		{
			Print(string.Format("Entity budget exceeded for player!"), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex);
			return;
		}
		
		SCR_EditableEntityComponent entity;
		bool hasRecipients = false;
		RplId currentLayerID;
		if (recipientIds && !recipientIds.IsEmpty())
		{
			//--- Spawn entity for selected entities (e.g., waypoints for groups)
			array<SCR_EditableEntityComponent> recipients = {};
			foreach (RplId id: recipientIds)
			{
				SCR_EditableEntityComponent recipient = SCR_EditableEntityComponent.Cast(Replication.FindItem(id));
				if (recipient)
					recipients.Insert(recipient);
			}
			array<vector> offsets = GetOffsets(recipients.Count());
			foreach (int i, SCR_EditableEntityComponent recipient: recipients)
			{
				params.m_Offset = offsets[i];
				entity = SpawnEntityResource(params, prefabResource, playerID, isQueue, recipient);
				entityIds.Insert(Replication.FindId(entity));
			}
			hasRecipients = true;
			currentLayerID = Replication.FindId(recipients[0].GetParentEntity());
		}
		else
		{
			//--- Spawn stand-alone entity
			entity = SpawnEntityResource(params, prefabResource, playerID, isQueue);
			entityIds.Insert(Replication.FindId(entity));
			currentLayerID = Replication.FindId(params.m_CurrentLayer);
		}
		Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, isQueue, hasRecipients, currentLayerID);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CreateEntityOwner(int prefabID, array<RplId> entityIds, int entityIndex, int isQueue, bool hasRecipients, RplId currentLayerID)
	{		
		//--- Delete the ghost preview which was waiting for server callback
		IEntity waitingPreview;
		if (m_WaitingPreviews.Find(entityIndex, waitingPreview))
		{
			m_WaitingPreviews.Remove(entityIndex);
			delete waitingPreview;
		}
		
		if (m_StatesManager && !m_StatesManager.SetIsWaiting(false))
		{
			SetSelectedPrefab();
			return;
		}
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		SCR_EditableEntityComponent entity;
		for (int i, count = entityIds.Count(); i < count; i++)
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityIds[i]));
			if (entity)
				entities.Insert(entity);
		}
		
		if (entities.IsEmpty())
		{
			SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
			Print(string.Format("Error when creating entity from prefab '%1' (id = %2)!", prefabData.GetPrefab(prefabID), prefabID), LogLevel.ERROR);
			return;
		}
		
		//--- Select placed entities
		SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		if (selectedFilter)
			selectedFilter.Replace(entities);
		
		//--- Set current layer to be the parent of newly created entity, to make sure its icon is visible
		SCR_EditableEntityComponent currentLayer = SCR_EditableEntityComponent.Cast(Replication.FindItem(currentLayerID));
		if (!currentLayerID.IsValid() || currentLayer)
		{
			SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
			if (layerManager)
				layerManager.SetCurrentLayer(currentLayer);
		}
		
		//--- Delayed check if entity is within budget (character budget update is delayed)
		if (isQueue)
		{
			GetGame().GetCallqueue().CallLater(CheckBudgetOwner, 100, false);
		}
		else
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		//--- Call event
		Event_OnPlaceEntity.Invoke(prefabID, entity);
		
		//--- Activate effects
		vector pos;
		entities[0].GetPos(pos);
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceConfirm(), this, pos, entities);
	}
	
	protected void CheckBudgetOwner()
	{
		// Check if prefab can still be placed, will invoke Event_OnBudgetMaxReached and cancel placing, see OnBudgetMaxReached
		EEditableEntityBudget blockingBudget;
		if (!CanSelectEntityPrefab(m_SelectedPrefab, blockingBudget, true))
		{
			m_BudgetManager.ResetPreviewCost();
		}
	}
	
	protected void OnBudgetMaxReached()
	{
		SetSelectedPrefab(ResourceName.Empty, true);
	}
	
	/*!
	Create entity from prefab resource.
	\param prefab Entity prefab
	\param transform Spawning transformation matrix
	\return Editable entity
	*/
	static SCR_EditableEntityComponent SpawnEntityResource(ResourceName prefab, vector transform[4])
	{
		return SpawnEntityResource(SCR_EditorPreviewParams.CreateParams(transform), Resource.Load(prefab));
	}
	/*!
	Create entity with given params.
	\param params Spawning params
	\prefabResource Prefab which will be spawned
	\playerID When valid ID, spawned entity will become the player
	\recipient Currently selected entity for whom the new entity is added (e.g., new waypoint for a group)
	\return Editable entity
	*/
	static SCR_EditableEntityComponent SpawnEntityResource(SCR_EditorPreviewParams params, Resource prefabResource, int playerID = 0, bool isQueue = false, SCR_EditableEntityComponent recipient = null)
	{
		if (Replication.IsClient() || !prefabResource|| !prefabResource.IsValid())
			return null;
		
		IEntity owner;
		EEditorPlacingFlags compatiblePlacingFlags = GetCompatiblePlacingFlags(prefabResource);
		if (playerID > 0)
		{
			//--- Spawn prefab as player
			if (SCR_Enum.HasFlag(compatiblePlacingFlags, EEditorPlacingFlags.CHARACTER_PLAYER))
			{
				SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
				if (respawnSystem)
				{
					owner = respawnSystem.CustomRespawn(playerID, prefabResource.GetResource().GetResourceName(), params.m_vTransform[3]/*, Math3D.MatrixToAngles(params.m_vTransform)*/);
				}
			}
		}
		else if (params.m_TargetInteraction == EEditableEntityInteraction.SLOT)
		{
			//--- Create in slot
			GenericEntity targetOwner = params.m_Target.GetOwner();
			SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(targetOwner);
			if (slot)
			{
				//--- Use special slot function
				owner = slot.SpawnEntityInSlot(prefabResource, Math3D.MatrixToAngles(params.m_vTransform)[0], -1/*verticalMode*/);
			}
			else
			{
				//--- Position on the target
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				targetOwner.GetWorldTransform(spawnParams.Transform);
				owner = GetGame().SpawnEntityPrefab(prefabResource, GetGame().GetWorld(), spawnParams);
			}
		}
		else
		{
			//--- Place freely
			//SCR_AIGroup.IgnoreSnapToTerrain(true);
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			params.GetWorldTransform(spawnParams.Transform);
			owner = GetGame().SpawnEntityPrefab(prefabResource, GetGame().GetWorld(), spawnParams);
		}
		
		//--- Initialize
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (!entity)
			return null;
		
		//--- Assign faction when placed for a recipient
		if (recipient)
			SCR_FactionControlComponent.SetFaction(entity.GetOwner(), recipient.GetFaction());
		
		//--- Call custom event
		SCR_EditableEntityComponent childEntity = entity.EOnEditorPlace(params.m_Parent, recipient, params.m_PlacingFlags, isQueue);
		
		//--- Set parent to current layer
		if (params.m_Parent)
			childEntity.SetParentEntity(params.m_Parent);
		
		//--- New parent was created in EOnEditorPlace, reflect that
		if (childEntity != entity)
			params.m_Parent = childEntity;
		
		//--- Decide which layer should be set as current
		params.m_CurrentLayer = childEntity.GetParentEntity();
		
		//--- Original entity deleted in SetParentEntity, use parent as the entity (e.g., when placing a group inside a group)
		if (!owner)
			entity = params.m_Parent;
	
		//--- Orient according to vertical settings
		if (!entity.HasEntityFlag(EEditableEntityFlag.STATIC_POSITION))
			SCR_RefPreviewEntity.SpawnAndApplyReference(entity, params);
		
		//--- Log message. Important for identifying problematic prefabs!
		vector logTransform[4];
		if (owner)
			owner.GetWorldTransform(logTransform);
		else
			params.GetWorldTransform(logTransform);
		
		if (recipient)
			Print(string.Format("@\"%1\" placed for %3 at %2", prefabResource.GetResource().GetResourceName().GetPath(), logTransform, recipient.GetDisplayName()), LogLevel.VERBOSE);
		else
			Print(string.Format("@\"%1\" placed at %2", prefabResource.GetResource().GetResourceName().GetPath(), logTransform), LogLevel.VERBOSE);
		
		return entity;
	}
	protected static EEditorPlacingFlags GetCompatiblePlacingFlags(Resource prefabResource)
	{
		IEntityComponentSource component = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
		switch (SCR_EditableEntityComponentClass.GetEntityType(component))
		{
			case EEditableEntityType.CHARACTER:
				return EEditorPlacingFlags.CHARACTER_PLAYER;
			
			case EEditableEntityType.VEHICLE:
				return EEditorPlacingFlags.VEHICLE_CREWED;
			
			case EEditableEntityType.TASK:
				return EEditorPlacingFlags.TASK_INACTIVE;
		}
		return 0;
	}
	
	protected bool CanPlaceEntityServer(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget)
	{
		SCR_BudgetEditorComponent budgetManagerOwner = SCR_BudgetEditorComponent.Cast(GetManager().FindComponent(SCR_BudgetEditorComponent));
		if (!budgetManagerOwner) return true;
		
		return budgetManagerOwner.CanPlaceEntitySource(editableEntitySource, blockingBudget, HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER), false);
	}
	
	protected bool CanSelectEntityPrefab(ResourceName prefab, out EEditableEntityBudget blockingBudget, bool showBudgetMaxNotification = true)
	{
		if (!m_BudgetManager || prefab.IsEmpty()) return true;
		
		// Load prefab to check entity budgets or entity type
		Resource entityPrefab = Resource.Load(prefab);
		// Get entity source
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		
		if (!entitySource)
		{
			Print("Could not determine budget cost for prefab: " +  prefab, LogLevel.WARNING);
			return false;
		}
		
		return m_BudgetManager.CanPlaceEntitySource(SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource), blockingBudget, HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER), true, showBudgetMaxNotification);
	}
	
	protected array<vector> GetOffsets(int count)
	{
		array<vector> result = {};
		if (count == 0)
			return result;
		
		if (count == 1)
		{
			result.Insert(vector.Zero);
			return result;
		}
		
		float m_fSpacing = 5; //--- ToDo: Member var
		int rowCount = Math.Round(Math.Sqrt(count));
		int columnCount = Math.Ceil(Math.Sqrt(count));
		vector offset = -Vector((rowCount - 1) * m_fSpacing / 2, 0, (columnCount - 1) * m_fSpacing / 2);

		int row, column;
		for (int i = 0; i < count; i++)
		{
			row = Math.Floor(i / columnCount);
			column = i % columnCount;
			result.Insert(offset + Vector(row * m_fSpacing, 0, column * m_fSpacing));
		}
		return result;
	}
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		int index = m_Recipients.Find(entity);
		if (index >= 0)
		{
			m_Recipients.Remove(index);
			
			//--- No remaining recipients, stop placing
			if (m_Recipients.IsEmpty())
				SetSelectedPrefab();
		}
	}
	
	/*!
	Set instant placing parameters.
	When defined, selected entity will be placed immediatelly.
	\param param Placing param
	*/
	void SetInstantPlacing(SCR_EditorPreviewParams param)
	{
		m_InstantPlacingParam = param;
	}
	
	/*!
	Set currently placed prefab.
	\param prefab Prefab. Must be registered!
	\return true when prefab can be selected (fits budget + prefab data available)
	*/
	bool SetSelectedPrefab(ResourceName prefab = "", bool onConfirm = false, bool showBudgetMaxNotification = true, set<SCR_EditableEntityComponent> recipients = null)
	{
		if (prefab == m_SelectedPrefab) return true;
		
		//--- Check if entity is within budget
		EEditableEntityBudget blockingBudget;
		if (!prefab.IsEmpty() && !CanSelectEntityPrefab(prefab, blockingBudget, showBudgetMaxNotification))
		{
			return false;
		}
		
		//--- Place instantly
		if (m_InstantPlacingParam && !prefab.IsEmpty())
		{
			m_SelectedPrefab = prefab;
			m_Recipients = recipients;
			CreateEntity();
			return true;
		}
		
		//--- Check if the prefab is registered
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefab.IsEmpty() && prefabData.GetPrefabID(prefab) == -1)
		{
			Print(string.Format("Cannot initiate placing of prefab @\"%1\", it's not registered in placeable entities!", prefab), LogLevel.WARNING);
			return false;
		}
		
		if (prefab.IsEmpty() || HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		if (m_StatesManager)
		{
			if (prefab)
			{
				if (!m_StatesManager.SetState(EEditorState.PLACING)) return false; //--- Exit when cannot set placing state
			}
			else
			{
				m_StatesManager.UnsetState(EEditorState.PLACING);
			}
		}
		
		//--- Track if recipients are deleted (stop placing when no recipient remains)
		if (prefab && recipients)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			core.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
		}
		else if (!prefab && m_Recipients)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			core.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
		}
		
		//--- Set the prefab
		ResourceName prefabPrev = m_SelectedPrefab;
		m_SelectedPrefab = prefab;
		m_Recipients = recipients;
		
		//--- Create preview entity
		int instanceCount = 1;
		if (recipients) instanceCount = recipients.Count();
		IEntity previewEntity = m_PreviewManager.CreatePreview(prefab, GetOffsets(instanceCount));//, fixedTransform);
		
		if (previewEntity)
		{
			//--- Teleport camera to fixed position
			if (m_PreviewManager.IsFixedPosition())
			{
				SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
				if (camera)
				{
					SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
					if (teleportComponent) teleportComponent.TeleportCamera(previewEntity.GetOrigin(), true, true);
				}
			}
			m_CompatiblePlacingFlags = GetCompatiblePlacingFlags(Resource.Load(m_SelectedPrefab));
		}
		else
		{
			SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
		}
		
		//--- Call event handlers
		Event_OnSelectedPrefabChange.Invoke(prefab, prefabPrev);
		
		if (prefab && !prefabPrev)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceStart(), this);
		else if (!prefab && prefabPrev && !onConfirm)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceCancel(), this);
		
		return true;
	}
	/*!
	Get currently placed prefab.
	\return prefab
	*/
	ResourceName GetSelectedPrefab()
	{
		return m_SelectedPrefab;
	}
	bool IsPlacing()
	{
		return !m_SelectedPrefab.IsEmpty();
	}
	/*!
	Attach plcing preview to a slot.
	\param slot Slot entity
	*/
	void SetSlot(SCR_SiteSlotEntity slot)
	{
		m_Slot = slot;
	}
	/*!
	Get slot which placing preview is currently attached to.
	\return Slot entity
	*/
	SCR_SiteSlotEntity GetSlot()
	{
		return m_Slot;
	}
	/*!
	Set value of placing flag.
	\param Placing flag
	\param toAdd True to set the flag, false to unset it
	*/
	void SetPlacingFlag(EEditorPlacingFlags flag, bool toAdd)
	{
		//--- When enabling, check if it's allowed (disabloing is always possible)
		if (toAdd && !IsPlacingFlagAllowed(flag))
		{
			Print(string.Format("Cannot enable placing flag %1, placing manager does not allow it!", typename.EnumToString(EEditorPlacingFlags, flag)), LogLevel.WARNING);
			return;
		}
		
		//--- Set the state
		EEditorPlacingFlags flagsBefore = m_PlacingFlags;
		if (toAdd)
			m_PlacingFlags |= flag;
		else
			m_PlacingFlags &= ~flag;
		
		//--- If there was a change, call event
		if (m_PlacingFlags != flagsBefore)
			Event_OnPlacingFlagsChange.Invoke(flag, toAdd);
		
		//--- Exception - update budget preview when placing player changes
		if (m_SelectedPrefab && flag == EEditorPlacingFlags.CHARACTER_PLAYER)
		{
			bool isPlacingPlayer = SCR_Enum.HasFlag(m_PlacingFlags, EEditorPlacingFlags.CHARACTER_PLAYER);
			EEditableEntityBudget blockingBudget;
			m_BudgetManager.CanPlaceEntitySource(SCR_EditableEntityComponentClass.GetEditableEntitySource(Resource.Load(m_SelectedPrefab).GetResource().ToEntitySource()), blockingBudget, isPlacingPlayer);
		}
	}
	/*!
	Toggle value of placing flag.
	\param flag Placing flag
	*/
	void TogglePlacingFlag(EEditorPlacingFlags flag)
	{
		SetPlacingFlag(flag, !HasPlacingFlag(flag));
	}
	/*!
	Check if placing flag is active.
	\param flag Placing flag
	\return True when active
	*/
	bool HasPlacingFlag(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_PlacingFlags, flag);
	}
	/*!
	Check if placing flag is compatible with currently placed entity.
	\param flag Placing flag
	\return True when compatible
	*/
	bool IsPlacingFlagCompatible(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_CompatiblePlacingFlags, flag);
	}
	/*!
	Check if placing flag is allowed in placing manager settings.
	\param flag Placing flag
	\return True when allowed
	*/
	bool IsPlacingFlagAllowed(EEditorPlacingFlags flag)
	{
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		return prefabData && prefabData.HasPlacingFlag(flag);
	}
	/*!
	Get invoker called when selected prefab changes.
	\return Script invoker
	*/	
	ScriptInvoker GetOnSelectedPrefabChange()
	{
		return Event_OnSelectedPrefabChange;
	}
	/*!
	Get event called when placing flags change.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnPlacingFlagsChange()
	{
		return Event_OnPlacingFlagsChange;
	}
	/*!
	Get event called when request for placing an entity is sent to server.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnRequestEntity()
	{
		return Event_OnRequestEntity;
	}
	/*!
	Get event called when an entity is placed.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnPlaceEntity()
	{
		return Event_OnPlaceEntity;
	}
	
	override void EOnEditorDebug(array<string> debugTexts)
	{
		if (!IsActive()) return;
		if (m_SelectedPrefab)
		{
			debugTexts.Insert(string.Format("Placing prefab: %1", FilePath.StripPath(m_SelectedPrefab.GetPath())));
		}
		else
		{
			debugTexts.Insert("Placing prefab: N/A");
		}
		debugTexts.Insert(string.Format("Placing flags: %1", SCR_Enum.FlagsToString(EEditorPlacingFlags, m_PlacingFlags)));
	}
	override void EOnEditorActivate()
	{
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		if (m_BudgetManager)
		{
			m_BudgetManager.Event_OnBudgetMaxReached.Insert(OnBudgetMaxReached);
		}
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ENABLE_AI_PILOT_PLACE, string.Empty, "Enable AI pilot", "Editable Entities");
	}
	
	override void EOnEditorDeactivate()
	{
		if (m_BudgetManager)
		{
			m_BudgetManager.Event_OnBudgetMaxReached.Remove(OnBudgetMaxReached);
		}
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ENABLE_AI_PILOT_PLACE);
	}
};