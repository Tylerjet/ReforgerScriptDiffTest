[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_PlaceableInventoryItemComponentClass : SCR_BaseInventoryItemComponentClass
{
}

class SCR_PlaceableInventoryItemComponent : SCR_BaseInventoryItemComponent
{
	[Attribute("1")]
	protected bool m_bSnapToGround;

	[Attribute("1", "Only works with Snap to ground")]
	protected bool m_bAlignToNormal;

	protected vector m_vMat[4];
	protected bool m_bUseTransform = false;
	protected RplId m_ParentRplId;
	protected int m_iParentNodeId;
	protected IEntity m_Parent;
	protected IEntity m_RootParent;

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	RplId GetParentRplId()
	{
		return m_ParentRplId;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int GetParentNodeId()
	{
		return m_iParentNodeId;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user
	// To be overridden, called when placement is done in SCR_ItemPlacementComponent
	void PlacementDone(notnull IEntity user);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] right
	//! \param[in] up
	//! \param[in] forward
	//! \param[in] position
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoPlaceItem(vector right, vector up, vector forward, vector position)
	{
		IEntity item = GetOwner();
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.EnablePhysics();
		itemComponent.ActivateOwner(true);

		m_vMat[0] = right;
		m_vMat[1] = up;
		m_vMat[2] = forward;
		m_vMat[3] = position;
		m_bUseTransform = true;

		PlayPlacedSound(up, position);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] right
	//! \param[in] up
	//! \param[in] forward
	//! \param[in] position
	void PlaceItem(vector right, vector up, vector forward, vector position)
	{
		Rpc(RPC_DoPlaceItem, right, up, forward, position);
		RPC_DoPlaceItem(right, up, forward, position);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	protected bool ValidateEntity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && (physics.IsDynamic() || physics.IsKinematic()))
			return false;

		// F. e. Slotted vehicle parts are physically static, but their main parent (vehicle) is not, we need to check that
		IEntity mainEntity = entity.GetRootParent();
		if (mainEntity && mainEntity != entity)
		{
			physics = mainEntity.GetPhysics();
			if (physics && physics.IsDynamic())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	protected bool FilterCallback(notnull IEntity e)
	{
		Physics physics = e.GetPhysics();
		if (physics)
		{
			if (physics.GetInteractionLayer() & EPhysicsLayerDefs.Water)
				return true;
		}

		return ValidateEntity(e);
	}

	//------------------------------------------------------------------------------------------------
	override bool OverridePlacementTransform(IEntity caller, out vector computedTransform[4])
	{
		ActivateOwner(true);

		IEntity owner = GetOwner();
		owner.Update();

		// Enable physics to receive contact events
		Physics physics = owner.GetPhysics();
		if (physics)
			EnablePhysics();

		if (m_bUseTransform)
		{
			// Entity was purposefully "placed" somewhere so we assume it should stay there (e.g. mines and flags to mark them)
			auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
			if (garbageSystem)
				garbageSystem.Withdraw(owner);

			Math3D.MatrixCopy(m_vMat, computedTransform);
			m_bUseTransform = false;
			return true;
		}

		if (m_bSnapToGround)
		{
			SCR_EntityHelper.SnapToGround(owner, {caller}, onlyStatic: true);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] up
	//! \param[in] position
	protected void PlayPlacedSound(vector up, vector position)
	{
		SCR_SoundDataComponent soundData;
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
		{
			soundData = SCR_SoundDataComponent.Cast(GetOwner().FindComponent(SCR_SoundDataComponent));
			if (!soundData)
				return;
		}

		TraceParam param = new TraceParam();
		param.Start = position;
		param.End = param.Start - up;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = GetOwner();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		GetOwner().GetWorld().TraceMove(param, null);

		GameMaterial material = param.SurfaceProps;
		if (!material)
			return;

		if (soundComp)
		{
			soundComp.SetSignalValueStr(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_PLACE_OBJECT);
		}
		else if (soundData)
		{
			SCR_SoundManagerEntity soundManager = GetGame().GetSoundManagerEntity();
			if (!soundManager)
				return;

			SCR_AudioSource soundSrc = soundManager.CreateAudioSource(GetOwner(), SCR_SoundEvent.SOUND_PLACE_OBJECT);
			if (!soundSrc)
				return;

			soundSrc.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			vector mat[4];	//due to the fact that item might still be in players inventory we need to override sound position
			mat[0] = up.Perpend();
			mat[1] = up;
			mat[2] = mat[0] * up;
			mat[3] = position;
			soundManager.PlayAudioSource(soundSrc, mat);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Same as PlaceItem but with params that allow attaching the object to new parent entity
	void PlaceItemWithParentChange(vector right, vector up, vector forward, vector position, RplId newParentRplId, int nodeId = -1)
	{
		Rpc(RPC_DoPlaceItemWithParentChange, right, up, forward, position, newParentRplId, nodeId);
		RPC_DoPlaceItemWithParentChange(right, up, forward, position, newParentRplId, nodeId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] right
	//! \param[in] up
	//! \param[in] forward
	//! \param[in] position
	//! \param[in] newParentRplId
	//! \param[in] nodeId
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoPlaceItemWithParentChange(vector right, vector up, vector forward, vector position, RplId newParentRplId, int nodeId)
	{
		IEntity item = GetOwner();
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.EnablePhysics();
		itemComponent.ActivateOwner(true);

		if (newParentRplId.IsValid())
		{
			m_ParentRplId = newParentRplId;
			m_iParentNodeId = nodeId;
			if (itemComponent.IsLocked())
				itemComponent.m_OnLockedStateChangedInvoker.Insert(AttachToNewParentWhenUnlocked);
			else
				AttachToNewParent();
		}
		else
		{
			m_ParentRplId = -1;
			m_iParentNodeId = -1;
		}

		m_vMat[0] = right;
		m_vMat[1] = up;
		m_vMat[2] = forward;
		m_vMat[3] = position;
		m_bUseTransform = true;

		PlayPlacedSound(up, position);
	}

	//------------------------------------------------------------------------------------------------
	//! Method that will try to find new parent entity and make this entity a child of it
	//! \param[in] nowLocked
	protected void AttachToNewParentWhenUnlocked(bool nowLocked)
	{
		if (nowLocked)
			return;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnLockedStateChangedInvoker.Remove(AttachToNewParentWhenUnlocked);

		AttachToNewParent();
	}

	//------------------------------------------------------------------------------------------------
	//! Method that will try to find new parent entity and make this entity a child of it
	protected void AttachToNewParent()
	{
		if (!m_ParentRplId.IsValid())
			return;

		RplComponent newParentRplComp = RplComponent.Cast(Replication.FindItem(m_ParentRplId));
		IEntity newParentEntity;
		if (newParentRplComp)
			m_Parent = newParentRplComp.GetEntity();	//cache it as depending how this item will be transfered later we may not have an easy access to it

		if (!m_Parent)
			m_Parent = IEntity.Cast(Replication.FindItem(m_ParentRplId));

		if (m_Parent)
			m_Parent.AddChild(GetOwner(), m_iParentNodeId);

		HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
		if (parentDamageManager)
		{
			SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
			if (hitZone)
				hitZone.GetOnDamageStateChanged().Insert(OnParentDamageStateChanged);
		}

		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(StartWatchingParentSlots);

		iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(DetachFromParent);

		if (m_Parent == m_Parent.GetRootParent())
			return;

		m_RootParent = m_Parent.GetRootParent();	//cache it as when this item will be transfered we may not be able to tell which one it was
		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(DetachFromParent);
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be added to the parent default hit zone in order to detect when it is destroyed
	//! \param[in] hitZone
	protected void OnParentDamageStateChanged(SCR_HitZone hitZone)
	{
		if (hitZone.GetDamageState() == EDamageState.DESTROYED)
			DetachFromParent();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes this entity from its parent hierarchy and places it on the ground
	protected void DetachFromParent()
	{
		if (!m_Parent)
			return;

		m_ParentRplId = -1;
		m_iParentNodeId = -1;
		m_Parent.RemoveChild(GetOwner(), true);
		array<IEntity> excludeArray = {m_Parent};
		if (m_RootParent)
			excludeArray.Insert(m_RootParent);

		SCR_EntityHelper.SnapToGround(GetOwner(), excludeArray, onlyStatic: true);
		GetOwner().SetAngles({0, GetOwner().GetAngles()[1], 0});
		GetOwner().Update();

		HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
		if (parentDamageManager)
		{
			SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
			if (hitZone)
				hitZone.GetOnDamageStateChanged().Remove(OnParentDamageStateChanged);
		}

		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(StopWatchingParentSlots);

		iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_Parent = null;
		if (!m_RootParent)
			return;

		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_RootParent = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered when item will finish its transfer to new parent when it is being attached to it
	protected void StartWatchingParentSlots()
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!iic)
			return;

		iic.m_OnParentSlotChangedInvoker.Remove(StartWatchingParentSlots);
		iic.m_OnParentSlotChangedInvoker.Insert(StopWatchingParentSlots);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method when item is being transfered (f.e. picked up) after it was attached to some object
	protected void StopWatchingParentSlots()
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(StopWatchingParentSlots);

		if (m_Parent)
		{
			HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
			if (parentDamageManager)
			{
				SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
				if (hitZone)
					hitZone.GetOnDamageStateChanged().Remove(OnParentDamageStateChanged);
			}

			iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
			if (iic)
				iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

			m_Parent = null;
		}

		if (!m_RootParent)
			return;

		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_RootParent = null;
	}
}
