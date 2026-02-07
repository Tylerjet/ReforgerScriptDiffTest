#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_DestructibleHitzone : ScriptedHitZone
{
	private DamageManagerComponent					m_ParentDamageManager; // Damage manager of the direct parent
	protected SCR_BaseCompartmentManagerComponent	m_pCompartmentManager; // The part may have occupants that we want to damage

	[Attribute("0", UIWidgets.EditBox, "Scale of received damage that will be passed to parent vehicle", "0 10 0.01")]
	private float m_fPassDamageToOwnerParent;

	[Attribute("", UIWidgets.Auto, desc: "Destruction Handler")]
	protected ref SCR_DestructionBaseHandler m_pDestructionHandler;

	[Attribute(desc: "Destruction sound event name", category: "Effects")]
	private string m_sDestructionSoundEvent;

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
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone.
	This is also called when transmitting the damage to parent hitzones!
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage source parent entity (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	override void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID)
	{
		super.OnDamage(type, damage, pOriginalHitzone, instigator, hitTransform, speed, colliderID, nodeID);

		if (this != pOriginalHitzone)
			return;

		if (IsProxy())
			return;

		// Skip delay when hit with strong ammunition and destruction is already scheduled
		if (damage > GetCriticalDamageThreshold()*GetMaxHealth() && m_pDestructionHandler && m_pDestructionHandler.IsDestructionQueued())
			m_pDestructionHandler.StartDestruction(true);

		if (m_fPassDamageToOwnerParent == 0)
			return;

		if (!m_ParentDamageManager)
			return;

		m_ParentDamageManager.HandleDamage(type, damage * m_fPassDamageToOwnerParent, hitTransform, m_ParentDamageManager.GetOwner(), m_ParentDamageManager.GetDefaultHitZone(), instigator, null, -1, -1);
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
			if (m_pDestructionHandler)
				m_pDestructionHandler.OnRepair();

			return;
		}

		SCR_HitZoneContainerComponent hitZoneContainer = SCR_HitZoneContainerComponent.Cast(GetHitZoneContainer());
		if (!hitZoneContainer)
			return;

		// Dont play sound when console app and when JIP
		if (!System.IsConsoleApp() && (!IsProxy() || hitZoneContainer.IsRplReady()))
			PlayDestructionSound(state);

		if (state != EDamageState.DESTROYED)
			return;

		SCR_VehicleDamageManagerComponent parentDamageManager = FindParentVehicleDamageManager();

		// Kill and eject occupants
		// Only main hitzone should deal damage to occupants.
		// TODO: Make passengers suffer random fire and bleeding instead, and let them get out if they are conscious.
		// TODO: Depends on ability to move the occupants to proper positions inside wrecks.
		if (hitZoneContainer.GetDefaultHitZone() == this && m_pCompartmentManager && !IsProxy())
		{
			SCR_DamageManagerComponent damageManager = parentDamageManager;
			if (!damageManager)
				damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());

			IEntity instigator;
			if (damageManager)
				instigator = damageManager.GetInstigatorEntity();

			m_pCompartmentManager.KillOccupants(instigator, true);
		}

		if (m_pDestructionHandler && !m_pDestructionHandler.IsDestructionQueued())
			m_pDestructionHandler.StartDestruction(false, parentDamageManager && parentDamageManager.IsInContact());
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
	protected void PlayDestructionSound(int damageState)
	{
		if (m_sDestructionSoundEvent.IsEmpty())
			return;

		if (damageState != EDamageState.DESTROYED)
			return;

		IEntity owner = GetOwner();
		if (!owner)
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
};
