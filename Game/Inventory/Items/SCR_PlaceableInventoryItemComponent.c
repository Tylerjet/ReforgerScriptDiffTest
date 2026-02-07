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
	void RPC_DoPlaceItem(vector right, vector up, vector forward, vector position)
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
	//! \param[out] normal
	//! \param[in] excludeArray
	//! \param[in] maxLength
	//! \param[in] direction
	void SnapToGround(out vector normal, array<IEntity> excludeArray = null, float maxLength = 10, vector startOffset = "0 0 0", vector direction = -vector.Up)
	{
		IEntity owner = GetOwner();
		vector origin = owner.GetOrigin();
		
		// Trace against terrain and entities to detect nearest ground
		TraceParam param = new TraceParam();
		param.Start = origin + startOffset;
		param.End = origin + direction * maxLength;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		if (excludeArray)
		{
			excludeArray.Insert(owner);
			param.ExcludeArray = excludeArray;
		}
		else
			param.Exclude = owner;
		
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance;
		
		traceDistance = world.TraceMove(param, FilterCallback);
		
		if (float.AlmostEqual(traceDistance, 1.0))
			return;
		
		normal = param.TraceNorm;
		
		owner.SetOrigin(traceDistance * (param.End - param.Start) + param.Start);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	bool ValidateEntity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && (physics.IsDynamic() || physics.IsKinematic()))
			return false;
		
		// F. e. Slotted vehicle parts are physically static, but their main parent (vehicle) is not, we need to check that
		IEntity mainEntity = SCR_EntityHelper.GetMainParent(entity);
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
	bool FilterCallback(notnull IEntity e)
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
			caller.GetTransform(computedTransform);
			vector normal = vector.Up;
			SnapToGround(normal, {caller});
			computedTransform[3] = owner.GetOrigin();
			
			if (m_bAlignToNormal)
				SCR_EntityHelper.OrientUpToVector(normal, computedTransform);
			
			return true;
		}

		return false;
	}
}
