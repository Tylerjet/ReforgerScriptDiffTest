#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
//! Base destruction handler, destruction handler types extend from this
//! Ported from SCR_DestructionBaseComponent
[BaseContainerProps()]
class SCR_DestructionBaseHandler
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Model to swap to on destruction", params: "xob")]
	private ResourceName m_sWreckModel;
	[Attribute("0", UIWidgets.EditBox, desc: "Delay for the wreck model switch upon destruction (ms)",  params: "0 10000 1")]
	private int m_iWreckDelay;
	[Attribute("100", UIWidgets.EditBox, desc: "Default mass of the wreck physics, to be set if the part did not have physics before destruction",  params: "0 10000 1")]
	private int m_fDefaultWreckMass;
	
	[Attribute("0", UIWidgets.CheckBox, "If true the part will detach from parent after hitzone is destroyed")]
	private bool m_bDetachAfterDestroyed;
	[Attribute("1", UIWidgets.CheckBox, "If true the part will be hidden after it is destroyed, unless wreck model is provided")]
	private bool m_bAllowHideWreck;
	[Attribute("1", UIWidgets.CheckBox, "If true and not detached the part will be deleted after parent entity is destroyed")]
	private bool m_bDeleteAfterParentDestroyed;
	[Attribute("0", UIWidgets.CheckBox, "If true the physics will be disabled after hitzone is destroyed")]
	private bool m_bDisablePhysicsAfterDestroyed;
#ifdef ENABLE_BASE_DESTRUCTION
	
	protected IEntity m_pOwner; // TODO: Remove once we can get the owner dynamically
	private bool m_bIsDestructionDelayed;
	
	bool IsDestructionQueued()
	{
		return m_bIsDestructionDelayed;
	}
	
	void StartDestruction(bool immediate = false, bool isInContact = false)
	{
		if (isInContact)
		{
			SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
			if (destructionManager)
			{
				destructionManager.DestroyInFrame(this, immediate);
				return;
			}
		}
		
		if (immediate || m_iWreckDelay <= 0)
		{
			if (m_bIsDestructionDelayed)
				GetGame().GetCallqueue().Remove(HandleDestruction);
			
			HandleDestruction();
		}
		else if (!m_bIsDestructionDelayed)
		{
			m_bIsDestructionDelayed = true;
			GetGame().GetCallqueue().CallLater(HandleDestruction, m_iWreckDelay);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Revert model back to default
	void OnRepair()
	{
		ResourceName object = SCR_Global.GetPrefabAttributeResource(m_pOwner, "MeshObject", "Object");
		SetModelResource(object);
		
		// Some objects have no valid destruction physics
		if (!m_bDisablePhysicsAfterDestroyed)
			return;
		
		m_pOwner.SetFlags(EntityFlags.TRACEABLE, false);
		
		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;
		
		// Set proper physics simulation state for slotted entities, use just collision physics.
		if (m_pOwner.GetParent())
			physics.ChangeSimulationState(SimulationState.COLLISION);
		else
			physics.ChangeSimulationState(SimulationState.SIMULATION);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle destruction
	protected void HandleDestruction()
	{
		m_bIsDestructionDelayed = false;
		if (!m_pOwner)
			return;
		
		// Destroy all children slotted entities
		array<EntitySlotInfo> slotInfos = {};
		EntitySlotInfo.GetSlotInfos(m_pOwner, slotInfos);
		foreach (EntitySlotInfo slotInfo: slotInfos)
		{
			if (!slotInfo)
				continue;

			IEntity slotEntity = slotInfo.GetAttachedEntity();
			if (!slotEntity)
				continue;

			HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(slotEntity.FindComponent(HitZoneContainerComponent));
			if (!hitZoneContainer)
				continue;
			
			HitZone hitZone = hitZoneContainer.GetDefaultHitZone();
			if (!hitZone)
				continue;
			
			// Need to trigger the destruction to hide or delete the unnecessary slotted objects
			if (hitZone.GetDamageState() == EDamageState.DESTROYED)
				hitZone.SetHealthScaled(1);
			
			hitZone.SetHealthScaled(0);
		}
		
		// Check parent only if the part is not set to be deleted anyway
		if (m_bDeleteAfterParentDestroyed && m_pOwner.GetParent())
		{
			HitZoneContainerComponent parentContainer = HitZoneContainerComponent.Cast(m_pOwner.GetParent().FindComponent(HitZoneContainerComponent));
			if (parentContainer && parentContainer.GetDefaultHitZone() && parentContainer.GetDefaultHitZone().GetDamageState() == EDamageState.DESTROYED)
			{
				DeleteSelf();
				return;
			}
		}
		
		if (m_bAllowHideWreck || !m_sWreckModel.IsEmpty())
			SetModelResource(m_sWreckModel, m_bAllowHideWreck);
		
		if (m_bDetachAfterDestroyed)
			DetachFromParent(true);
		
		// Some objects have no valid destruction physics
		if (!m_bDisablePhysicsAfterDestroyed)
			return;
		
		m_pOwner.ClearFlags(EntityFlags.TRACEABLE, false);
		
		Physics physics = m_pOwner.GetPhysics();
		if (physics)
			physics.ChangeSimulationState(SimulationState.NONE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether provided entity has parent and if so, tries to find a slot which it would belong to
	//! and unregister from it.
	private void DetachFromParent(bool updateHierarchy)
	{
		if (!m_pOwner)
			return;
		
		if (!m_pOwner.GetParent())
			return;
		
		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;
		
		vector velocity = physics.GetVelocity();
		vector angularVelocity = physics.GetAngularVelocity();
		
		EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(m_pOwner);
		if (slotInfo)
			slotInfo.DetachEntity(updateHierarchy);
		
		if (physics.IsDynamic() && !m_pOwner.GetParent())
		{
			physics.SetVelocity(velocity);
			physics.SetAngularVelocity(angularVelocity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deletes self
	void DeleteSelf()
	{
		if (!m_pOwner)
			return;
		
		if (m_pOwner.GetParent())
			DetachFromParent(false);
		
		m_pOwner.SetObject(null, string.Empty);
		RplComponent.DeleteRplEntity(m_pOwner, false);
	}

	//------------------------------------------------------------------------------------------------
	/*! Sets the model of the object
	\param resourceName Path to model
	\param allowHide Allow setting empty model
	*/
	protected void SetModelResource(ResourceName resourceName, bool allowEmpty = false)
	{
		VObject model;
		if (!resourceName.IsEmpty())
		{
			Resource resource = Resource.Load(resourceName);
			if (resource.IsValid())
				model = resource.GetResource().ToVObject();
		}
		
		if (model || allowEmpty)
			SetModel(model);
	}
	//------------------------------------------------------------------------------------------------
	/*! Sets the model of the object
	\param model Loaded model data
	*/
	protected void SetModel(VObject model)
	{
		Vehicle vehicle = Vehicle.Cast(m_pOwner);
		if (vehicle)
			vehicle.SetWreckModel(model);
		else if (m_pOwner)
			m_pOwner.SetObject(model, string.Empty);
		
		m_pOwner.Update();
		
		if (!model)
			return;
		
		// If there is no model, ignore the rest
		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;
		
		// If the object has dynamic physics, pass the state
		float mass = physics.GetMass();
		if (mass == 0)
			mass = m_fDefaultWreckMass;
		
		//int layerMask = physics.GetInteractionLayer();
		if (physics.IsDynamic())
		{
			vector velocity = physics.GetVelocity();
			vector angularVelocity = physics.GetAngularVelocity();
			physics.Destroy();
			
			physics = Physics.CreateDynamic(m_pOwner, mass, 0xffffffff);
			if (physics && !m_pOwner.GetParent())
			{
				physics.SetVelocity(velocity);
				physics.SetAngularVelocity(angularVelocity);
			}
		}
		else
		{
			physics = Physics.CreateStatic(m_pOwner, 0xffffffff);
			if (physics)
				physics.SetMass(mass);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner, HitZone hitZone)
	{
		m_pOwner = owner;
	}
#endif
};