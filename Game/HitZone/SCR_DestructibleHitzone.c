//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Visual destruction effects should be handled via separate component called by damage manager, not hitzone
#define ENABLE_BASE_DESTRUCTION
class SCR_DestructibleHitzone : SCR_HitZone
{
	[Attribute(desc: "Secondary explosion reference point", category: "Secondary damage")]
	protected ref PointInfo m_SecondaryExplosionPoint;

	[Attribute("", UIWidgets.Auto, desc: "Destruction Handler")]
	protected ref SCR_DestructionBaseHandler m_pDestructionHandler;

	[Attribute(desc: "Destruction sound event name", category: "Effects")]
	protected string m_sDestructionSoundEvent;

	[Attribute(desc: "Particle effect for destruction", params: "ptc", category: "Effects")]
	protected ResourceName m_sDestructionParticle;

	[Attribute(uiwidget: UIWidgets.Coords, "Position of the effect in model space", category: "Effects")]
	protected vector m_vParticleOffset;

	protected ParticleEffectEntity					m_DstParticle; // Destruction particle
	protected SCR_DamageManagerComponent			m_ParentDamageManager; // Damage manager of the direct parent
	protected SCR_DamageManagerComponent			m_RootDamageManager;
	protected SCR_BaseCompartmentManagerComponent	m_CompartmentManager; // The part may have occupants that we want to damage

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

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));

		// Parent damage manager
		IEntity parent = pOwnerEntity.GetParent();
		if (parent)
			m_ParentDamageManager = SCR_DamageManagerComponent.Cast(parent.FindComponent(SCR_DamageManagerComponent));

		// Root damage manager
		IEntity root = pOwnerEntity.GetRootParent();
		if (root)
			m_RootDamageManager = SCR_DamageManagerComponent.Cast(root.FindComponent(SCR_DamageManagerComponent));

		if (m_pDestructionHandler)
			m_pDestructionHandler.Init(pOwnerEntity, this);
		
		if (m_SecondaryExplosionPoint)
			m_SecondaryExplosionPoint.Init(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	float GetSecondaryExplosionScale()
	{
		return GetMaxHealth();
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	PointInfo GetSecondaryExplosionPoint()
	{
		return m_SecondaryExplosionPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Kill occupants and start destruction
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		EDamageState state = GetDamageState();

		// Restrict AI from accessing this parts compartments
		if (m_CompartmentManager)
			m_CompartmentManager.SetCompartmentsAccessibleForAI(state != EDamageState.DESTROYED);

		// Change from destroyed can only mean repair
		if (GetPreviousDamageState() == EDamageState.DESTROYED)
		{
			StopDestruction();
			return;
		}

		if (state == EDamageState.DESTROYED)
			StartDestruction();

		PlayDestructionSound(state);
		PlayDestructionParticle(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction effects
	void StartDestruction(bool immediate = false)
	{
		if (m_CompartmentManager && GetHitZoneGroup() == EVehicleHitZoneGroup.HULL)
			m_CompartmentManager.EjectRandomOccupants(-1, true);
		
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
		SCR_DamageManagerComponent hitZoneContainer = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());
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

	//------------------------------------------------------------------------------------------------
	//! Start destruction particle
	void PlayDestructionParticle(EDamageState state)
	{
		// Particles are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (state == EDamageState.DESTROYED && !m_DstParticle && !m_sDestructionParticle.IsEmpty())
		{
			ParticleEffectEntitySpawnParams spawnParams();
			spawnParams.Transform[3] = m_vParticleOffset;
			spawnParams.Parent = GetOwner();
			spawnParams.UseFrameEvent = true;
			m_DstParticle = ParticleEffectEntity.SpawnParticleEffect(m_sDestructionParticle, spawnParams);
		}
	}
#endif
}
//---- REFACTOR NOTE END ----
