#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_DestructibleHitzone : ScriptedHitZone
{
	protected DamageManagerComponent				m_ParentDamageManager; // Damage manager of the direct parent
	protected SCR_BaseCompartmentManagerComponent	m_pCompartmentManager; // The part may have occupants that we want to damage

	[Attribute("0", UIWidgets.Slider, "Scale of damage passed to parent default hitzone\nIgnores base damage multiplier, reduction and threshold\nDamage type multipliers are applied", "0 100 0.001")]
	protected float m_fPassDamageToParentScale;

	[Attribute("", UIWidgets.Auto, desc: "Destruction Handler")]
	protected ref SCR_DestructionBaseHandler m_pDestructionHandler;

	[Attribute(desc: "Destruction sound event name", category: "Effects")]
	protected string m_sDestructionSoundEvent;

#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	ref SCR_DestructionBaseHandler GetDestructionHandler()
	{
		return m_pDestructionHandler;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		m_pCompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));

		// Parent damage manager
		IEntity parent = pOwnerEntity.GetParent();
		if (parent)
			m_ParentDamageManager = DamageManagerComponent.Cast(parent.FindComponent(DamageManagerComponent));

		if (m_pDestructionHandler)
			m_pDestructionHandler.Init(pOwnerEntity, this);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculates the amount of damage a hitzone will receive.
	\param damageType - damage type
	\param rawDamage - incoming damage, without any modifiers taken into account
	\param hitEntity - damaged entity
	\param struckHitZone - hitzone to damage
	\param damageSource - projectile
	\param damageSourceGunner - damage source instigator
	\param damageSourceParent - damage source parent entity (soldier, vehicle)
	\param hitMaterial - hit surface physics material
	\param colliderID - collider ID - if it exists
	\param hitTransform - hit position, direction and normal
	\param impactVelocity - projectile velocity in time of impact
	\param nodeID - bone index in mesh obj
	\param isDOT - true if this is a calculation for DamageOverTime
	*/
	override float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, IEntity damageSourceGunner, IEntity damageSourceParent, const GameMaterial hitMaterial, int colliderID, inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		// Forward non-DOT damage to parent, ignoring own base damage multiplier
		if (!isDOT)
			PassDamageToParent(damageType, rawDamage, damageSourceParent, hitTransform, hitMaterial);

		return super.ComputeEffectiveDamage(damageType, rawDamage, hitEntity, struckHitZone, damageSource, damageSourceGunner, damageSourceParent, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Pass damage to default hitzone of parent damage manager, ignoring base damage multiplier, reduction and threshold
	\param type - damage type, use TRUE damage to ignore thresholds and multipliers of default hitzone
	\param damage - amount of damage passed to default hitzone of parent damage manager, multiplied by damage type multiplier of current hitzone
	\param instigator - instigator entity
	\param transform - hit position, direction and normal
	\param surface - hit surface properties
	*/
	void PassDamageToParent(EDamageType type, float damage, IEntity instigator, inout vector transform[3], SurfaceProperties surface = null)
	{
		if (!m_ParentDamageManager)
			return;

		damage *= m_fPassDamageToParentScale * GetDamageMultiplier(type);
		if (damage == 0)
			return;

		m_ParentDamageManager.HandleDamage(type, damage, transform, m_ParentDamageManager.GetOwner(), m_ParentDamageManager.GetDefaultHitZone(), instigator, surface, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! Kill occupants and start destruction
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();

		EDamageState state = GetDamageState();

		// Restrict AI from accessing this parts compartments
		if (m_pCompartmentManager)
			m_pCompartmentManager.SetCompartmentsAccessibleForAI(state != EDamageState.DESTROYED);

		// Change from destroyed can only mean repair
		if (GetPreviousDamageState() == EDamageState.DESTROYED)
		{
			StopDestruction();
			return;
		}

		if (state == EDamageState.DESTROYED)
			StartDestruction();

		PlayDestructionSound(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction effects
	void StartDestruction(bool immediate = false)
	{
		SCR_VehicleDamageManagerComponent parentDamageManager = FindParentVehicleDamageManager();

		// Kill and eject occupants
		// Only main hitzone should deal damage to occupants.
		// TODO: Make passengers suffer random fire and bleeding instead, and let them get out if they are conscious.
		// TODO: Depends on ability to move the occupants to proper positions inside wrecks.
		HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		if (hitZoneContainer && hitZoneContainer.GetDefaultHitZone() == this && m_pCompartmentManager && !IsProxy())
		{
			SCR_DamageManagerComponent damageManager = parentDamageManager;
			if (!damageManager)
				damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());

			IEntity instigator;
			if (damageManager)
				instigator = damageManager.GetInstigatorEntity();

			m_pCompartmentManager.KillOccupants(instigator, true);
		}

		if (m_pDestructionHandler)
			m_pDestructionHandler.StartDestruction(immediate);
	}

	//------------------------------------------------------------------------------------------------
	//! Stop destruction effects
	protected void StopDestruction()
	{
		// Must not change model outside frame
		if (m_pDestructionHandler)
			GetGame().GetCallqueue().CallLater(m_pDestructionHandler.OnRepair);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_VehicleDamageManagerComponent FindParentVehicleDamageManager()
	{
		// Futile if there is no hitzonecontainer anymore
		HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		if (!hitZoneContainer)
			return null;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(hitZoneContainer);
		if (damageManager)
			return damageManager;

		damageManager = SCR_VehicleDamageManagerComponent.Cast(m_ParentDamageManager);
		if (damageManager)
			return damageManager;

		IEntity parent = GetOwner().GetParent();
		while (parent)
		{
			damageManager = SCR_VehicleDamageManagerComponent.Cast(parent.FindComponent(SCR_VehicleDamageManagerComponent));
			if (damageManager)
				return damageManager;

			parent = parent.GetParent();
		}

		return damageManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction sounds
	protected void PlayDestructionSound(EDamageState damageState)
	{
		// Sounds are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (damageState != EDamageState.DESTROYED)
			return;

		if (m_sDestructionSoundEvent.IsEmpty())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		// Play destruction effects only if not streaming in
		SCR_HitZoneContainerComponent hitZoneContainer = SCR_HitZoneContainerComponent.Cast(GetHitZoneContainer());
		if (IsProxy() && hitZoneContainer && !hitZoneContainer.IsRplReady())
			return;

		IEntity mainParent = SCR_EntityHelper.GetMainParent(owner);
		if (mainParent)
		{
			// Get position offset for slotted entities
			vector mins;
			vector maxs;
			owner.GetBounds(mins, maxs);
			vector center = vector.Lerp(mins, maxs, 0.5);

			// Add bone offset
			EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(owner);
			if (slotInfo)
			{
				vector mat[4];
				slotInfo.GetModelTransform(mat);
				center = center + mat[3];
			}

			SoundComponent soundComponent = SoundComponent.Cast(mainParent.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEventOffset(m_sDestructionSoundEvent, center);
		}
		else
		{
			SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEvent(m_sDestructionSoundEvent);
		}
	}
#endif
}
