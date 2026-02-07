enum ECharacterDamageState: EDamageState
{
	WOUNDED = 3
};

enum ECharacterHealthState: EDamageState
{
	MODERATE = 3,
	SERIOUS = 4,
	CRITICAL = 5
};

enum ECharacterBloodState: EDamageState
{
	WEAKENED = 3,
	FAINTING = 4,
	UNCONSCIOUS = 5
};

enum ECharacterResilienceState : EDamageState
{
	WEAKENED = 3,
	FAINTING = 4,
	UNCONSCIOUS = 5
};

enum EBandagingAnimationBodyParts
{
	Invalid = 0,
	UpperHead = 1,
	LowerHead = 2,
	UpperTorso = 3,
	LowerTorso = 4,
	LeftHand = 5,
	RightHand = 6,
	LeftLeg = 7,
	RightLeg = 8
};

class SCR_BleedingHitZoneParameters : Managed
{
	void SCR_BleedingHitZoneParameters(
		HitZone hitZone,
		float DamageOverTime	
	)
	{
		m_hHitZone = hitZone;
		m_fDamageOverTime = DamageOverTime;
	}
	HitZone m_hHitZone;
	float m_fDamageOverTime;
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterHitZone : SCR_RegeneratingHitZone
{
	protected static const float DAMAGE_TO_BLOOD_MULTIPLIER = 1.3; //Const value is based on previous testing by caliber
	
	[Attribute("0.25", UIWidgets.Auto, "Multiplier of hitzone health that will be applied as bleeding damage over time when damaged")]
	protected float m_fBleedingRateScale;
	
	[Attribute()]
	protected ref array<string> m_aDamageSubmeshes;
	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref LoadoutAreaType> m_aBleedingAreas;

	[Attribute(ECharacterHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;	
	
	[Attribute(EBandagingAnimationBodyParts.Invalid.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EBandagingAnimationBodyParts))]
	protected EBandagingAnimationBodyParts m_eBandageAnimationBodyPart;
	
	protected bool m_bIsWounded;
	
	//-----------------------------------------------------------------------------------------------------------
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
		
		// Update last instigator
		SCR_DamageManagerComponent manager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.SetInstigatorEntity(instigator);
		
		// Only serious hits should cause bleeding
		if (damage < GetCriticalDamageThreshold()*GetMaxHealth())
			return;
		
		if (Math.RandomFloat(0,1) < 0.3)
			return;
		
		// Adding immediately some blood to the clothes - currently it's based on the damage dealt.
		AddBloodToClothes(Math.Clamp(damage * DAMAGE_TO_BLOOD_MULTIPLIER, 0, 255));
		AddBleeding(colliderID);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Whether hitzone submeshes are hidden with clothing
	bool IsCovered()
	{
		CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetOwner().FindComponent(CharacterIdentityComponent));
		if (!identity)
			return false;
		
		foreach (string submesh: m_aDamageSubmeshes)
		{
			if (identity.IsCovered(submesh))
				return true;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Add bleeding to this hitzone provided with collider ID
	This should be called from RPC from server on all clients and the host
	\param colliderID ID of physics collider to attach particle effects to Default: -1 (random)
	*/
	void AddBleeding(int colliderID = -1)
	{
		if (!HasColliderNodes())
			return;
		
		int colliderDescriptorIndex = GetColliderDescriptorIndex(colliderID);
		if (colliderDescriptorIndex < 0)
		{
			int currentHealth = GetHealth();
			colliderDescriptorIndex = currentHealth % GetNumColliderDescriptors();
		}
		
		AddBleedingToCollider(colliderDescriptorIndex);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Add bleeding to this hitzone provided with collider descriptor index
	This should be called from RPC from server on all clients and the host
	\param colliderDescriptorIndex Index off the colldier descriptor in this hitzone
	*/
	void AddBleedingToCollider(int colliderDescriptorIndex = -1)
	{
		if (!HasColliderNodes())
			return;
		
		// Currently we only have one bleeding per hitzone
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.AddBleedingHitZone(this, colliderDescriptorIndex);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();

		UpdateSubmeshes();
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.UpdateGroupDamage(GetHitZoneGroup());
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void AddBloodToClothes(float immediateBloodEffect = 0)
	{
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(GetOwner().FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return;
		
		IEntity clothEntity;
		ParametricMaterialInstanceComponent materialComponent;
		for (int i = m_aBleedingAreas.Count() - 1; i >= 0; i--)
		{
			clothEntity = loadoutStorage.GetClothFromArea(m_aBleedingAreas[i].Type());
			if (!clothEntity)
				continue;
			
			materialComponent = ParametricMaterialInstanceComponent.Cast(clothEntity.FindComponent(ParametricMaterialInstanceComponent));
			if (!materialComponent)
				continue;
			
			if (immediateBloodEffect > 0)
			{
				materialComponent.SetUserParam2(Math.Clamp(materialComponent.GetUserParam2() + immediateBloodEffect, 1, 255));
				continue;
			}
			
			materialComponent.SetUserParam2(Math.Clamp(materialComponent.GetUserParam2() + GetMaxBleedingRate() * 0.1, 1, 255));
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetWoundedSubmesh(bool wounded)
	{
		m_bIsWounded = wounded;
		
		CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetOwner().FindComponent(CharacterIdentityComponent));
		if (!identity)
			return;
		
		foreach (string submesh: m_aDamageSubmeshes)
			identity.SetWoundState(submesh, wounded);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Manage wounds submeshes
	void UpdateSubmeshes()
	{
		bool isWounded = GetDamageStateThreshold(GetDamageState()) <= GetDamageStateThreshold(ECharacterDamageState.WOUNDED);
		if (m_bIsWounded != isWounded)
			SetWoundedSubmesh(isWounded);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Maximum bleeding rate that this hitzone may apply to Blood HitZone
	\return bleedingRate Maximum bleeding rate that this hitzone may apply to Blood HitZone
	*/
	float GetMaxBleedingRate()
	{
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!manager)
			return 1;
		
		float bleedingRate = m_fBleedingRateScale * GetMaxHealth();
		
		bleedingRate *= manager.GetDOTScale();
		
		return bleedingRate;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	EBandagingAnimationBodyParts GetBodyPartToHeal()
	{
		return m_eBandageAnimationBodyPart;
	}

	//-----------------------------------------------------------------------------------------------------------
	override void UpdatePassiveRegenerationOverTime()
	{
		super.UpdatePassiveRegenerationOverTime();
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
			if (!damageMgr)
				return;
		
		if (!damageMgr.CanPassivelyRegenerate())
		{
			SetDamageOverTime(EDamageType.REGENERATION, 0);	
			return;
		}
		
		if (!GetDamageOverTime(EDamageType.BLEEDING) && GetDamageOverTime(EDamageType.FIRE) <= 0)
			SetDamageOverTime(EDamageType.REGENERATION, (m_fPassivelyRegeneratingAmount * damageMgr.GetPassiveRegenerationStanceMultiplier()));	
	}
};

//-----------------------------------------------------------------------------------------------------------
class SCR_RegeneratingHitZone : ScriptedHitZone
{
	[Attribute("10", UIWidgets.Auto, "Time without receiving damage or bleeding to start regeneration [s]")]
	protected float m_fRegenerationDelay;
	[Attribute("200", UIWidgets.Auto, "Time to fully regenerate resilience hitzone [s]")]
	protected float m_fFullRegenerationTime;

	protected float m_fPassivelyRegeneratingAmount;

	//-----------------------------------------------------------------------------------------------------------
	/*!
	Called when damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone. This is also called when 
	transmitting the damage to parent hitzones!
	\param damage Amount of damage received
	\param type Type of damage
	\param pHitEntity Entity that was damaged
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage instigator
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param colliderID ID of the collider receiving damage
	\param speed Projectile speed in time of impact
	*/
	override void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID)
	{
		super.OnDamage(type, damage, pOriginalHitzone, instigator, hitTransform, speed, colliderID, nodeID);
		
		if (IsProxy())
			return;
		
		RemovePassiveRegeneration();
		ScheduleRegeneration();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnHealthSet()
	{
		super.OnHealthSet();
		
		if (IsProxy())
			return;
		
		RemovePassiveRegeneration();
		ScheduleRegeneration();
	}
	
	// Calculate amount of passive regeneration
	float CalculatePassiveRegenAmount()
	{
		if (m_fFullRegenerationTime <= 0)
			return 0; 
		
		return -GetMaxHealth() / m_fFullRegenerationTime;
	}
	
	// Schedule healing over time, delay current scheduled regeneration
	void ScheduleRegeneration()
	{
		GetGame().GetCallqueue().Remove(UpdatePassiveRegenerationOverTime);
		GetGame().GetCallqueue().CallLater(UpdatePassiveRegenerationOverTime, 1000 * m_fRegenerationDelay, true);
	}
	
	// Remove current healing over time and any scheduled regeneration
	void RemovePassiveRegeneration()
	{
		GetGame().GetCallqueue().Remove(UpdatePassiveRegenerationOverTime);
		
		if (GetDamageOverTime(EDamageType.REGENERATION))
		{
			SetDamageOverTime(EDamageType.REGENERATION, 0);
			m_fPassivelyRegeneratingAmount = 0;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	protected void UpdatePassiveRegenerationOverTime()
	{
		if (IsProxy())
			return;
		
		if (GetDamageState() == ECharacterDamageState.UNDAMAGED)
			return;
		
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		foreach (EDamageType damageType: damageTypes)
		{
			if (damageType == EDamageType.REGENERATION || damageType == EDamageType.FIRE)
				continue;
			
			if (GetDamageOverTime(damageType) != 0)
				return;
		}
				
		// TODO@FAC: SCR_PatientCompartmentSlot regenerationBoost 
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (character)
		{
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			float medicalVehicleRegenMultiplier = 1;
			if (compAccess)
			{
				SCR_PatientCompartmentSlot patientSlot = SCR_PatientCompartmentSlot.Cast(compAccess.GetCompartment());
				if (patientSlot)
					medicalVehicleRegenMultiplier = patientSlot.GetCompartmentRegenRateMultiplier();
			}
			
			SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
			if (!damageMgr)
				return;
			
			m_fPassivelyRegeneratingAmount = (medicalVehicleRegenMultiplier * CalculatePassiveRegenAmount()) * damageMgr.GetRegenScale();
		}
		// ENDTODO
		
		SetDamageOverTime(EDamageType.REGENERATION, m_fPassivelyRegeneratingAmount);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		if (GetDamageState() == ECharacterDamageState.UNDAMAGED)
			RemovePassiveRegeneration();
	}
};

//-----------------------------------------------------------------------------------------------------------
//! Resilience - incapacitation or death, depending on game mode settings
class SCR_CharacterResilienceHitZone : SCR_RegeneratingHitZone
{
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageManager)
			return;
		
		damageManager.UpdateConsciousness();
		
		// If destroyed and unconsciousness is not allowed, kill character
		if (GetDamageState() != EDamageState.DESTROYED || damageManager.GetPermitUnconsciousness())
			return;

		damageManager.Kill(damageManager.GetInstigatorEntity());
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
		if (characterDamageManager)
			characterDamageManager.SetResilienceHitZone(this);
	}
};

//-----------------------------------------------------------------------------------------------------------
//! Blood - does not receive damage directly, only via scripted events.
class SCR_CharacterBloodHitZone : SCR_RegeneratingHitZone
{
	ref map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters> m_mHitZoneDOTMap;
	ref array<float> m_aHitZoneGroupBleedings;
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageManager)
			return;
		
		damageManager.UpdateConsciousness();
		
		if (GetDamageState() == ECharacterBloodState.DESTROYED)
			damageManager.Kill(damageManager.GetInstigatorEntity());
		
		// If destroyed and unconsciousness is not allowed, kill character
		if (GetDamageStateThreshold(GetDamageState()) > GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))
			return;

		if (damageManager.GetPermitUnconsciousness())
			return;
		
		damageManager.Kill(damageManager.GetInstigatorEntity());
	}

	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.SetBloodHitZone(this);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool AddBleedingHZToMap(SCR_CharacterHitZone hitZone, SCR_BleedingHitZoneParameters localBleedingHZParams)
	{
		if (!m_mHitZoneDOTMap)
			m_mHitZoneDOTMap = new map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters>();

		if (m_mHitZoneDOTMap.Contains(hitZone))
			m_mHitZoneDOTMap.Remove(hitZone);

		return m_mHitZoneDOTMap.Insert(hitZone, localBleedingHZParams);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void RemoveBleedingHZFromMap(SCR_CharacterHitZone hitZone)
	{
		if (!m_mHitZoneDOTMap)
			return;
		
		m_mHitZoneDOTMap.Remove(hitZone);
		
		if (m_mHitZoneDOTMap.IsEmpty())
			m_mHitZoneDOTMap = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	float GetPhysicalHZBleedingDOT(notnull SCR_CharacterHitZone hitZone)
	{
		if (!m_mHitZoneDOTMap || m_mHitZoneDOTMap.IsEmpty())
			return 0;
		
		SCR_BleedingHitZoneParameters localBleedingHZParams;
		
		if (m_mHitZoneDOTMap.Find(hitZone, localBleedingHZParams))
			return localBleedingHZParams.m_fDamageOverTime;
		
		return 0;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters> GetHitZoneDOTMap()
	{
		return m_mHitZoneDOTMap;
	}
};

//-----------------------------------------------------------------------------------------------------------
class SCR_CharacterHeadHitZone : SCR_CharacterHitZone
{
	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
		if (manager)
			manager.SetHeadHitZone(this);
	}
};
