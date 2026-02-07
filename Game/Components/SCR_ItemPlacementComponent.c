[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_ItemPlacementComponentClass : ScriptComponentClass
{
}

class SCR_ItemPlacementComponent : ScriptComponent
{
	[Attribute("{56EBF5038622AC95}Assets/Conflict/CanBuild.emat")]
	protected ResourceName m_sCanBuildMaterial;

	[Attribute("{14A9DCEA57D1C381}Assets/Conflict/CannotBuild.emat")]
	protected ResourceName m_sCannotBuildMaterial;

	[Attribute("{8FBC3A6E946F056E}Common/Materials/Default_Transparent.emat")]
	protected ResourceName m_sTransparentMaterial;

	protected int m_iEquipComplete;
	protected int m_iTargetEntityNodeID;
	protected bool m_bInEditor;
	protected bool m_bIsBeingAttachedToEntity;
	protected vector m_vCurrentMat[4];
	protected vector m_aCamDeploymentPosition[4];
	protected IEntity m_EquippedItem;
	protected ENotification m_eCantPlaceReason;
	protected SCR_PlaceableItemComponent m_PlaceableItem;
	protected IEntity m_PreviewEntity;
	protected SCR_CompartmentAccessComponent m_CompartmnetAccessComponent;
	protected RplId m_TargetId;
	protected IEntity m_TargetEntity;

	//------------------------------------------------------------------------------------------------
	//! Client side method for informing the authority about where item should be placed upon removal from their inventory
	//! \param[in] right
	//! \param[in] up
	//! \param[in] forward
	//! \param[in] position
	//! \param[in] placeableId replication id of placed item
	//! \param[in] characterId replication id of the character that placed it there
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetPlacementPosition(vector right, vector up, vector forward, vector position, RplId placeableId, RplId characterId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		if (!characterId.IsValid())
			return;

		itemComponent.SetPlacementPosition(right, up, forward, position, characterId);
	}

	//------------------------------------------------------------------------------------------------
	//! placeableId = id of items rpl component
	//! \param[in] placeableId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPlaceItem(RplId placeableId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.PlaceItem();
	}

	//------------------------------------------------------------------------------------------------
	//! placeableId = id of items rpl component
	//! \param[in] placeableId
	//! \param[in] targetId is an RplId of the owner of the surface to which this item is meant to be attached to
	//! \param[in] nodeId is an node id of the surface to which this object is meant to be attached to
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPlaceItemWithParentChange(RplId placeableId, RplId targetId, int nodeId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		if (targetId.IsValid())
			itemComponent.PlaceItemWithParentChange(targetId, nodeId);
		else
			itemComponent.PlaceItem();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent GetCharacterController(IEntity from)
	{
		if (!from)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(from);
		if (!character)
			return SCR_CharacterControllerComponent.Cast(from.FindComponent(SCR_CharacterControllerComponent));

		return SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterEvents(IEntity from)
	{
		if (!from)
			return;

		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(from.FindComponent(BaseWeaponManagerComponent));
		if (weaponManager)
		{
			weaponManager.m_OnWeaponChangeCompleteInvoker.Remove(OnWeaponChangeEnded);
			weaponManager.m_OnWeaponChangeStartedInvoker.Remove(OnWeaponChanged);
		}

		SCR_CharacterControllerComponent characterController = GetCharacterController(from);
		if (characterController)
			characterController.GetOnPlayerDeathWithParam().Remove(OnCharacterDeath);

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(from.FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccessComponent)
		{
			compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnComparmentEntered);
			compartmentAccessComponent.GetOnCompartmentLeft().Remove(GetOnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterEvents(IEntity to)
	{
		if (!to)
			return;

		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(to.FindComponent(BaseWeaponManagerComponent));
		if (weaponManager)
		{
			weaponManager.m_OnWeaponChangeCompleteInvoker.Insert(OnWeaponChangeEnded);
			weaponManager.m_OnWeaponChangeStartedInvoker.Insert(OnWeaponChanged);
		}

		SCR_CharacterControllerComponent characterController = GetCharacterController(to);
		if (characterController)
			characterController.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccessComponent)
		{
			compartmentAccessComponent.GetOnCompartmentEntered().Insert(OnComparmentEntered);
			compartmentAccessComponent.GetOnCompartmentLeft().Insert(GetOnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_EquippedItem = null;
		ChimeraCharacter characterTo = ChimeraCharacter.Cast(to);

		if (characterTo)
		{
			m_CompartmnetAccessComponent = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
			m_EquippedItem = characterTo.GetCharacterController().GetRightHandItem();
		}
		else
		{
			m_CompartmnetAccessComponent = null;
		}

		// unregister from previous event handler
		UnregisterEvents(from);

		// register to new event handler
		RegisterEvents(to);
	}

	//------------------------------------------------------------------------------------------------
	protected void StartPlaceItem()
	{
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return;

		if (m_eCantPlaceReason > 0)
		{
			SCR_NotificationsComponent.SendLocal(m_eCantPlaceReason);
			return;
		}

		SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
		if (!characterController)
			return;

		characterController.SetDisableWeaponControls(true);
		characterController.SetDisableMovementControls(true);

		if (characterController.IsUsingItem())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		CharacterAnimationComponent animationComponent = character.GetAnimationComponent();
		int itemActionId = animationComponent.BindCommand("CMD_Item_Action");
		vector mat[4];
		Math3D.MatrixCopy(m_vCurrentMat, mat);
		PointInfo ptWS = new PointInfo();
		mat[2] = (mat[3] - character.GetOrigin()).Normalized();
		mat[1] = vector.Up;
		mat[0] = Vector(mat[2][2], mat[2][1], -mat[2][0]);
		ptWS.Set(null, "", mat);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(m_EquippedItem);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(false);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(1);
		params.SetMaxAnimLength(15.0);
		params.SetAlignmentPoint(ptWS);

		if (characterController.TryUseItemOverrideParams(params))
		{
			characterController.m_OnItemUseEndedInvoker.Insert(OnPlacingEnded);
			DisablePreview();
		}
		characterController.GetAnimationComponent().GetCommandHandler().AlignNewTurns();

		if (m_bIsBeingAttachedToEntity)
			GetGame().GetCallqueue().CallLater(ValidateTargetEntityExistance, 500, true);	//used to locally check if the entity to which player wants to attach the object is still there
	}

	//------------------------------------------------------------------------------------------------
	void OnPlacingEnded(IEntity item, bool successful, ItemUseParameters animParams)
	{
		GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);

		ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!controlledEntity)
			return;

		SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
		if (!characterController)
			return;

		RplComponent characterRplComp = controlledEntity.GetRplComponent();
		if (!characterRplComp)
			return;

		characterController.SetDisableWeaponControls(true);
		characterController.SetDisableMovementControls(true);

		characterController.m_OnItemUseEndedInvoker.Remove(OnPlacingEnded);

		if (!successful)
		{
			EnablePreview(item);
			return;
		}

		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(controlledEntity.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = SCR_CharacterInventoryStorageComponent.Cast(controlledEntity.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storage)
			return;

		RplComponent rplComponent = RplComponent.Cast(m_EquippedItem.FindComponent(RplComponent));
		if (!rplComponent)
			return;

		SCR_PlaceableInventoryItemComponent placeableItem = SCR_PlaceableInventoryItemComponent.Cast(m_EquippedItem.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (placeableItem)
		{
			SCR_PlacementCallback cb = new SCR_PlacementCallback();
			cb.m_PlaceableId = rplComponent.Id();
			cb.m_TargetId = m_TargetId;
			cb.m_iNodeId = m_iTargetEntityNodeID;
			cb.m_bIsBeingAttachedToEntity = m_bIsBeingAttachedToEntity;

			Rpc(RPC_AskSetPlacementPosition, m_vCurrentMat[0], m_vCurrentMat[1], m_vCurrentMat[2], m_vCurrentMat[3], rplComponent.Id(), characterRplComp.Id());
			if (storageManager.TryRemoveItemFromStorage(m_EquippedItem, storage.GetWeaponStorage(), cb))
			{
				InventoryItemComponent equippedItemIIC = InventoryItemComponent.Cast(m_EquippedItem.FindComponent(InventoryItemComponent));
				if (equippedItemIIC && equippedItemIIC.GetAttributes())
				{
					InventoryStorageSlot parentSlot = equippedItemIIC.GetParentSlot();
					int slotId = -1;
					if (parentSlot && EquipedWeaponStorageComponent.Cast(parentSlot.GetStorage()))
						slotId = parentSlot.GetID();

					ECommonItemType equippedItemType = equippedItemIIC.GetAttributes().GetCommonType();
					if (equippedItemType != ECommonItemType.NONE)
					{
						array<IEntity> ownedItems = {};
						SCR_CommonItemTypeSearchPredicate itemSearch = new SCR_CommonItemTypeSearchPredicate(equippedItemType, m_EquippedItem);
						storageManager.FindItems(ownedItems, itemSearch);
						if (ownedItems.Count())
						{
							storage.RemoveItemFromQuickSlotAtIndex(slotId);
							storage.StoreItemToQuickSlot(ownedItems[0], slotId);
						}
					}
				}
			}
		}

		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	//! Client side method for finishing item placement when that item was successfully removed from their inventory
	//! \param[in] placeableId replication id of placed item
	//! \param[in] targetId replication id of object to which this item is being attached to
	//! \param[in] nodeId node of the target object to which placed item will be attached to
	//! \param[in] isBeingAttachedToEntity
	void AskPlaceItem(RplId placeableId, RplId targetId, int nodeId, bool isBeingAttachedToEntity)
	{
		if (!m_bIsBeingAttachedToEntity)
			Rpc(RPC_AskPlaceItem, placeableId);
		else if (targetId.IsValid())
			Rpc(RPC_AskPlaceItemWithParentChange, placeableId, targetId, nodeId);
	}

	//------------------------------------------------------------------------------------------------
	void ValidateTargetEntityExistance()
	{
		TraceParam param = new TraceParam();
		param.Start = m_aCamDeploymentPosition[3];
		param.End = param.Start + m_aCamDeploymentPosition[2] * m_PlaceableItem.GetMaxPlacementDistance();
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = GetOwner().GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		IEntity tracedEntity = param.TraceEnt;

		if (!tracedEntity && m_TargetId.IsValid())
		{
			m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_SURFACE_NO_LONGER_THERE;//entity to which we were attaching is no longer there
		}
		else if (tracedEntity)
		{
			RplComponent rplComp = RplComponent.Cast(tracedEntity.FindComponent(RplComponent));
			if (!rplComp || rplComp.Id() != m_TargetId)
				m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_SURFACE_NO_LONGER_THERE;//entity to which we were attaching is no longer there
			else if (m_iTargetEntityNodeID != param.NodeIndex)
				m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_DIFFERENT_SURFACE;//same entity but different part of it is now there
			else
				m_eCantPlaceReason = 0;
		}

		if (m_eCantPlaceReason > 0)
		{
			IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (!controlledEntity)
				return;

			SCR_CharacterControllerComponent characterController = GetCharacterController(controlledEntity);
			if (!characterController)
				return;

			if (!characterController.IsUsingItem())
			{
				GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);
			}
			else
			{
				CharacterAnimationComponent animationComponent = characterController.GetAnimationComponent();
				if (!animationComponent)
					return;

				CharacterCommandHandlerComponent commandHandler = animationComponent.GetCommandHandler();
				commandHandler.CancelItemUse();
				SCR_NotificationsComponent.SendLocal(m_eCantPlaceReason);
				GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DisablePreview()
	{
		GetGame().GetInputManager().RemoveActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
		delete m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnablePreview(IEntity weapon)
	{
		m_PlaceableItem = SCR_PlaceableItemComponent.Cast(weapon.FindComponent(SCR_PlaceableItemComponent));

		EntityPrefabData prefabData = weapon.GetPrefabData();
		if (!prefabData)
			return;

		GetGame().GetInputManager().AddActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);

		SetEventMask(GetOwner(), EntityEvent.FRAME);
		if (!m_PreviewEntity)
			m_PreviewEntity = GetGame().SpawnEntity(GenericEntity, GetOwner().GetWorld());
			//m_PreviewEntity = SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(Resource.Load(prefabData.GetPrefabName()), "SCR_BasePreviewEntity");
			// Use SCR_PrefabPreviewEntity API once we have api for hierarchi bbox

		SCR_PlaceableItemComponent placeableItemComponent = SCR_PlaceableItemComponent.Cast(weapon.FindComponent(SCR_PlaceableItemComponent));
		if (!placeableItemComponent)
		{
			Print("SCR_PlaceableItemComponent not found! SCR_ItemPlacementComponent.EnablePreview()");
			return;
		}

		vector test = m_PreviewEntity.GetOrigin();
		m_PreviewEntity.SetObject(placeableItemComponent.GetPreviewVobject(), "");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComparmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void GetOnCompartmentLeft()
	{
		TogglePreview(m_EquippedItem);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponComponent
	//! \return
	IEntity GetWeaponFromWeaponComponent(BaseWeaponComponent weaponComponent)
	{
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weaponComponent);
		if (!weaponSlot)
			return null;

		return weaponSlot.GetWeaponEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeaponChangeEnded(BaseWeaponComponent newWeaponSlot)
	{
		m_EquippedItem = GetWeaponFromWeaponComponent(newWeaponSlot);
		TogglePreview(m_EquippedItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeaponChanged(BaseWeaponComponent newWeaponSlot)
	{
		DisablePreview(); // Automatically disable preview to handle cases where anim event could be missing
		m_EquippedItem = GetWeaponFromWeaponComponent(newWeaponSlot);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCharacterDeath(SCR_CharacterControllerComponent characterController, IEntity killerEntity, notnull Instigator killer)
	{
		UnregisterEvents(killerEntity);
		m_EquippedItem = null; // Reset this cached value
		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	// Reacts to weapon changes of local character
	protected void TogglePreview(IEntity weapon)
	{
		if (m_bInEditor || !weapon || !weapon.FindComponent(SCR_PlaceableItemComponent))
		{
			DisablePreview();
		}
		else
		{
			if (weapon.GetRootParent() != SCR_PlayerController.GetLocalControlledEntity())
				return;

			EnablePreview(weapon);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_CompartmnetAccessComponent.IsGettingIn())
		{
			DisablePreview();
			return;
		}

		if (!m_EquippedItem)
		{
			// This handles situations where f. e. land mines explode in characters hands
			DisablePreview();
			return;
		}

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase currentCamera = cameraManager.CurrentCamera();
		if (!currentCamera)
			return;

		vector cameraMat[4];
		currentCamera.GetTransform(cameraMat);
		float maxPlacementDistance = m_PlaceableItem.GetMaxPlacementDistance();
		SCR_EPlacementType placementType = m_PlaceableItem.GetPlacementType();

		m_eCantPlaceReason = 0;
		switch (placementType)
		{
			case SCR_EPlacementType.XZ_FIXED:
				UseXZFixedPlacement(owner, maxPlacementDistance, cameraMat);
				break;

			case SCR_EPlacementType.XYZ:
				UseXYZPlacement(owner, maxPlacementDistance, cameraMat);
				break;
		}

		if (m_eCantPlaceReason == 0)
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCanBuildMaterial);
		else if (m_eCantPlaceReason == ENotification.PLACEABLE_ITEM_CANT_PLACE_DISTANCE)
			SCR_Global.SetMaterial(m_PreviewEntity, m_sTransparentMaterial);
		else
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] maxPlacementDistance
	//! \param[in] cameraMat
	void UseXZFixedPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		vector direction = cameraMat[2];
		direction[1] = 0;
		direction.Normalize();

		IEntity character = SCR_PlayerController.GetLocalControlledEntity();

		// Trace against terrain and entities to detect item placement position
		TraceParam param = new TraceParam();
		param.Start = character.GetOrigin() + maxPlacementDistance * direction + vector.Up;
		param.End = param.Start - 5 * vector.Up;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = character;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;

		SCR_EntityHelper.OrientUpToVector(up, m_vCurrentMat);
		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();

		ValidatePlacement(up, param.TraceEnt, world, character, m_eCantPlaceReason);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterCallback(IEntity e)
	{
		Physics physics = e.GetPhysics();
		if (physics)
		{
			if ((physics.IsDynamic() || physics.IsKinematic()) && !m_PlaceableItem.CanAttachToDynamicObject())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] maxPlacementDistance
	//! \param[in] cameraMat
	void UseXYZPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		// Trace against terrain and entities to detect item placement position
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		TraceParam param = new TraceParam();
		param.Start = character.EyePosition();
		param.End = param.Start + cameraMat[2] * maxPlacementDistance;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;
		m_vCurrentMat[3] = m_vCurrentMat[3] + up * 0.01;

		IEntity tracedEntity = param.TraceEnt;

		if (traceDistance == 1) // Assume we didn't hit anything and snap item on the ground
		{
			// Trace against terrain and entities to detect new placement position
			TraceParam paramGround = new TraceParam();
			paramGround.Start = param.End + vector.Up;
			paramGround.End = paramGround.Start - vector.Up * 20;
			paramGround.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			paramGround.Exclude = SCR_PlayerController.GetLocalControlledEntity();
			paramGround.LayerMask = EPhysicsLayerPresets.Projectile;
			float traceGroundDistance = world.TraceMove(paramGround, FilterCallback);
			m_PreviewEntity.GetTransform(m_vCurrentMat);
			m_vCurrentMat[3] = paramGround.Start + ((paramGround.End - paramGround.Start) * traceGroundDistance) + vector.Up * 0.01; // adding 1 cm to avoid collision with object under

			if (traceGroundDistance < 1)
				up = paramGround.TraceNorm;

			tracedEntity = paramGround.TraceEnt;
		}

		SCR_EntityHelper.OrientUpToVector(up, m_vCurrentMat);
		vector right = up * cameraMat[0];
		vector forward = up * right;
		right.Normalize();
		forward.Normalize();
		m_vCurrentMat[0] = forward;
		m_vCurrentMat[2] = right;

#ifdef WORKBENCH
		ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
		Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + up * 0.5, 0.1, Color.BLUE, shapeFlags);
		Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + right * 0.5, 0.1, Color.ORANGE, shapeFlags);
		Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + forward * 0.5, 0.1, Color.GREEN, shapeFlags);
#endif

		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();

		// Reject based on distance from character
		if (SCR_PlaceableItemComponent.GetDistanceFromCharacter(character, m_vCurrentMat[3], m_PlaceableItem.GetDistanceMeasurementMethod()) > maxPlacementDistance)
		{
			m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_DISTANCE;//this position is too far away
			return;
		}

		m_iTargetEntityNodeID = param.NodeIndex;
		m_aCamDeploymentPosition = cameraMat;
		m_aCamDeploymentPosition[3] = character.EyePosition();
		ValidatePlacement(m_vCurrentMat[1], tracedEntity, world, SCR_PlayerController.GetLocalControlledEntity(), m_eCantPlaceReason);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	protected bool ValidateEntity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && physics.IsDynamic() && !m_PlaceableItem.CanAttachToDynamicObject())
			return false;

		// F. e. Slotted vehicle parts are physically static, but their main parent (vehicle) is not, we need to check that
		IEntity mainEntity = entity.GetRootParent();
		if (mainEntity && mainEntity != entity)
		{
			physics = mainEntity.GetPhysics();
			if (physics && physics.IsDynamic() && !m_PlaceableItem.CanAttachToDynamicObject())
				return false;
		}

		IEntity parent = entity;
		array<typename> ignoredComponents = {};
		m_PlaceableItem.GetIgnoredComponents(ignoredComponents);
		while (parent)
		{
			physics = parent.GetPhysics();
			if (physics && (physics.GetInteractionLayer() & m_PlaceableItem.GetIgnoredPhysicsLayers()))
				return false;

			if (ignoredComponents.IsEmpty())
			{
				parent = parent.GetParent();
				continue;
			}

			foreach (typename comp : ignoredComponents)
			{
				if (parent.FindComponent(comp))
					return false;
			}
			parent = parent.GetParent();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] up
	//! \param[in] tracedEntity
	//! \param[in] world
	//! \param[in] character
	//! \return
	void ValidatePlacement(vector up, IEntity tracedEntity, BaseWorld world, IEntity character, out ENotification cantPlaceReason)
	{
		if (!m_PlaceableItem.CanBeAttachedWhileAngled() && vector.Dot(up, vector.Up) < 0.5) // Early reject based on the angle of placement (the maximum should be dictated by item settings)
		{
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
			cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_TOO_STEEP;//angle too steep
			return;
		}

		if (tracedEntity != m_TargetEntity)
		{
			m_bIsBeingAttachedToEntity = false;
			m_TargetEntity = null;
			m_TargetId = -1;
			m_iTargetEntityNodeID = -1;

			if (!ValidateEntity(tracedEntity)) // Reject on items with dynamic physics
			{
				cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_GENERIC;//cant attach to this
				return;
			}

			Physics entPhys = tracedEntity.GetPhysics();
			if (entPhys && !(entPhys.GetInteractionLayer() & EPhysicsLayerDefs.Terrain))
				m_bIsBeingAttachedToEntity = true;
		}

		array<IEntity> excludeArray = {};
		excludeArray.Insert(m_PreviewEntity);
		excludeArray.Insert(character);

		// Reject based on bbox collision with surrounding objects - last check, as it's the most expensive one
		// Check bbox
		TraceOBB paramOBB = new TraceOBB();
		Math3D.MatrixIdentity3(paramOBB.Mat);
		paramOBB.Mat[0] = m_vCurrentMat[0];
		paramOBB.Mat[1] = m_vCurrentMat[1];
		paramOBB.Mat[2] = m_vCurrentMat[2];
		paramOBB.Start = m_vCurrentMat[3] + 0.05 * paramOBB.Mat[1];
		paramOBB.Flags = TraceFlags.ENTS;
		paramOBB.ExcludeArray = excludeArray;
		paramOBB.LayerMask = EPhysicsLayerPresets.Projectile;
		m_PreviewEntity.GetBounds(paramOBB.Mins, paramOBB.Maxs);
		world.TracePosition(paramOBB, TraceEntitiesCallback);

#ifdef WORKBENCH
		ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
		Shape.CreateArrow(paramOBB.Start, paramOBB.Start + paramOBB.Mat[1] * 0.5, 0.1, Color.WHITE, shapeFlags);
		Shape.CreateArrow(paramOBB.Start, paramOBB.Start + paramOBB.Mat[2] * 0.5, 0.1, Color.PINK, shapeFlags);
		Shape.CreateArrow(paramOBB.Start, paramOBB.Start + paramOBB.Mat[0] * 0.5, 0.1, Color.YELLOW, shapeFlags);
#endif

		// collides with another entity
		if (paramOBB.TraceEnt)
		{
			cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_NOT_ENOUGH_SPACE;//not enough space
			return;
		}

		if (tracedEntity && m_bIsBeingAttachedToEntity)
		{
			RplComponent rplComp = RplComponent.Cast(tracedEntity.FindComponent(RplComponent));
			if (rplComp)
				m_TargetId = rplComp.Id();
			else
				m_bIsBeingAttachedToEntity = false;
		}
		m_TargetEntity = tracedEntity;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	bool TraceEntitiesCallback(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpened()
	{
		m_bInEditor = true;
		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		m_bInEditor = false;
		TogglePreview(m_EquippedItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterToManagerInvokers(SCR_EditorManagerEntity manager)
	{
		manager.GetOnOpened().Insert(OnEditorOpened);
		manager.GetOnClosed().Insert(OnEditorClosed);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitEditorListeners()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();

		if (!editorManager)
			core.Event_OnEditorManagerInitOwner.Insert(RegisterToManagerInvokers);
		else
			RegisterToManagerInvokers(editorManager);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
		{
			Print("SCR_ItemPlacementComponent is not attached to SCR_PlayerController!", LogLevel.WARNING);
			return;
		}

		Math3D.MatrixIdentity4(m_vCurrentMat);

		InitEditorListeners();

		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ItemPlacementComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iEquipComplete = GameAnimationUtils.RegisterAnimationEvent("EquipComplete");
	}
}
