[EntityEditorProps(category: "GameScripted/Components", description: "ScriptWizard generated script file.")]
class SCR_ItemPlacementComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ItemPlacementComponent : ScriptComponent
{
	[Attribute("{56EBF5038622AC95}Assets/Conflict/CanBuild.emat")]
	ResourceName m_sCanBuildMaterial;
	
	[Attribute("{14A9DCEA57D1C381}Assets/Conflict/CannotBuild.emat")]
	ResourceName m_sCannotBuildMaterial;
	
	[Attribute("5")]
	int m_iFramesBetweenChecks;
	
	protected int m_iEquipComplete;
	protected bool m_bCanPlace;
	protected vector m_vCurrentMat[4];
	protected IEntity m_EquippedItem;
	protected SCR_PlaceableItemComponent m_PlaceableItem;
	protected IEntity m_PreviewEntity;
	
	//------------------------------------------------------------------------------------------------
	//! id = id of items rpl component
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPlaceItem(vector right, vector up, vector forward, vector position, RplId id)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(id));
		if (!rplComponent)
			return;
		
		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;
		
		itemComponent.PlaceItem(right, up, forward, position);
	}
	
	//------------------------------------------------------------------------------------------------
	// Listens to animation events of character
	protected void OnAnimEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (animEventType == m_iEquipComplete)
		{
			OnEquipComplete();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEquipComplete()
	{
		TogglePreview(m_EquippedItem);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		EventHandlerManagerComponent eventHandler;
		SCR_CharacterControllerComponent characterController;
		
		// unregister from previous event handler
		if (from)
		{
			eventHandler = EventHandlerManagerComponent.Cast(from.FindComponent(EventHandlerManagerComponent));
			if (eventHandler)
				eventHandler.RemoveScriptHandler("OnWeaponChanged", this, OnWeaponChanged);
			
			characterController = SCR_CharacterControllerComponent.Cast(from.FindComponent(SCR_CharacterControllerComponent));
			if (characterController)
				characterController.GetOnAnimationEvent().Remove(OnAnimEvent);
		}
		
		// register to new event handler
		if (to)
		{
			eventHandler = EventHandlerManagerComponent.Cast(to.FindComponent(EventHandlerManagerComponent));
			if (eventHandler)
				eventHandler.RegisterScriptHandler("OnWeaponChanged", this, OnWeaponChanged);
			
			characterController = SCR_CharacterControllerComponent.Cast(to.FindComponent(SCR_CharacterControllerComponent));
			if (characterController)
				characterController.GetOnAnimationEvent().Insert(OnAnimEvent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StartPlaceItem()
	{
		if (!m_bCanPlace)
			return;
		
		IEntity controlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		if (!controlledEntity)
			return;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		if (!characterController)
			return;
		
		if (characterController.IsUsingItem())
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		CharacterAnimationComponent animationComponent = character.GetAnimationComponent();
		int itemActionId = animationComponent.BindCommand("CMD_Item_Action");
		PointInfo ptWS = new PointInfo();
		m_vCurrentMat[2] = (m_vCurrentMat[3] - character.GetOrigin()).Normalized();
		m_vCurrentMat[1] = vector.Up;
		m_vCurrentMat[0] = Vector(m_vCurrentMat[2][2], m_vCurrentMat[2][1], -m_vCurrentMat[2][0]);
		ptWS.Set(null, "", m_vCurrentMat);
		if (characterController.TryUseItemOverrideParams(m_EquippedItem, false, itemActionId, 1, 0, 15.0, 1, 0.0, false, ptWS))
		{
			characterController.m_OnItemUseEndedInvoker.Insert(OnPlacingEnded);
			DisablePreview();
		}
		characterController.GetAnimationComponent().GetCommandHandler().AlignNewTurns();
		
		RplComponent rplComponent = RplComponent.Cast(m_EquippedItem.FindComponent(RplComponent));
		if (!rplComponent)
			return;
		
		SCR_PlaceableInventoryItemComponent mineItem = SCR_PlaceableInventoryItemComponent.Cast(m_EquippedItem.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (mineItem)
			Rpc(RPC_AskPlaceItem, m_vCurrentMat[0], m_vCurrentMat[1], m_vCurrentMat[2], m_vCurrentMat[3], rplComponent.Id());
			//mineItem.PlaceItem(m_vCurrentMat[0], m_vCurrentMat[1], m_vCurrentMat[2], m_vCurrentMat[3]);
	}
	
	//------------------------------------------------------------------------------------------------
	/*protected*/ void OnPlacingEnded(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams)
	{
		IEntity controlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		if (!controlledEntity)
			return;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		if (!characterController)
			return;
		
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
		
		storageManager.TryRemoveItemFromStorage(m_EquippedItem, storage.GetWeaponStorage());
		
		DisablePreview();
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
	protected void OnWeaponChanged(BaseWeaponComponent weapon, BaseWeaponComponent prevWeapon)
	{
		DisablePreview(); // Automatically disable preview to handle cases where anim event could be missing
		
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weapon);
		if (!weaponSlot)
			return;
		
		IEntity weaponEntity = weaponSlot.GetWeaponEntity();
		
		m_EquippedItem = weaponEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	// Reacts to weapon changes of local character
	protected void TogglePreview(IEntity weapon)
	{
		if (!weapon || !weapon.FindComponent(SCR_PlaceableItemComponent))
			DisablePreview();
		else
			EnablePreview(weapon);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_EquippedItem)
		{
			// This handles situations where f. e. land mines explode in characters hands
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
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
		
		switch (placementType)
		{
			case SCR_EPlacementType.XZ_FIXED:
				UseXZFixedPlacement(owner, maxPlacementDistance, cameraMat);
				break;
			
			case SCR_EPlacementType.XYZ:
				UseXYZPlacement(owner, maxPlacementDistance, cameraMat);
				break;
		}
		
		if (m_bCanPlace)
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCanBuildMaterial);
		else
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
	}
	
	//------------------------------------------------------------------------------------------------
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
		float traceDistance = world.TraceMove(param, null);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;
		
		OrientToNormal(up);
		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();
		
		m_bCanPlace = ValidatePlacement(up, param.TraceEnt, world, character);
	}
	
	//------------------------------------------------------------------------------------------------
	void UseXYZPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		// Trace against terrain and entities to detect item placement position
		TraceParam param = new TraceParam();
		param.Start = cameraMat[3];
		param.End = param.Start + cameraMat[2] * maxPlacementDistance;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, null);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;
		
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
			float traceGroundDistance = world.TraceMove(paramGround, null);
			m_PreviewEntity.GetTransform(m_vCurrentMat);
			m_vCurrentMat[3] = paramGround.Start + ((paramGround.End - paramGround.Start) * traceGroundDistance);
			
			if (traceGroundDistance < 1)
				up = paramGround.TraceNorm;
			
			if (vector.Distance(m_vCurrentMat[3], cameraMat[3]) < maxPlacementDistance)
				tracedEntity = paramGround.TraceEnt;
		}
		
		OrientToNormal(up);
		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();
		
		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		vector characterOrigin = character.GetOrigin();
		
		if (Math.AbsFloat(m_vCurrentMat[3][1] - characterOrigin[1] > 0.2)) // Reject based on vertical distance from character
		{
			m_bCanPlace = false;
			return;
		}
		
		if (vector.Distance(m_vCurrentMat[3], characterOrigin) > maxPlacementDistance) // Reject based on distance from character (the maximum should be dictated by item settings)
		{
			m_bCanPlace = false;
			return;
		}
		
		m_bCanPlace = ValidatePlacement(up, tracedEntity, world, SCR_PlayerController.GetLocalControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	bool ValidatePlacement(vector up, IEntity tracedEntity, BaseWorld world, IEntity character)
	{
		if (vector.Dot(up, vector.Up) < 0.5) // Early reject based on the angle of placement (the maximum should be dictated by item settings)
		{
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
			return false;
		}
		
		if (tracedEntity) // Reject on items with dynamic physics
		{
			Physics physics = tracedEntity.GetPhysics();
			if (physics && physics.IsDynamic())
				return false;
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
		paramOBB.Start = m_vCurrentMat[3] + "0 0.03 0";
		paramOBB.Flags = TraceFlags.ENTS;
		paramOBB.ExcludeArray = excludeArray;
		paramOBB.LayerMask = EPhysicsLayerPresets.Projectile;
		m_PreviewEntity.GetBounds(paramOBB.Mins, paramOBB.Maxs);
		world.TracePosition(paramOBB, TraceEntitiesCallback);
		
		// collides with another entity
		if (paramOBB.TraceEnt)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool TraceEntitiesCallback(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OrientToNormal(vector normal)
	{
		vector origin = m_vCurrentMat[3];
		vector perpend = normal.Perpend();
		Math3D.DirectionAndUpMatrix(perpend, normal, m_vCurrentMat);
		
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(-perpend.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(m_vCurrentMat, basis, m_vCurrentMat);
		m_vCurrentMat[3] = origin;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
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
		GetGame().GetInputManager().AddActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);
		
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ItemPlacementComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iEquipComplete = GameAnimationUtils.RegisterAnimationEvent("EquipComplete");
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ItemPlacementComponent()
	{
	}

};
