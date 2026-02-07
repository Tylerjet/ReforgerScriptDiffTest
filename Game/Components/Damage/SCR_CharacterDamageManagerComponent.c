//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponentClass: ScriptedDamageManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponent : ScriptedDamageManagerComponent
{
	// 5000 ms timer for bloody face update
	static const int BLOODY_FACE_UPDATE_PERIOD = 5000;
	
	// 1000 ms timer for bloody clothes update
	static const int BLOOD_CLOTHES_UPDATE_PERIOD = 1000;
	
	private SCR_CharacterBloodHitZone m_pBloodHitZone;
	private ref array<HitZone> m_aBleedingHitZones;
	private ref map<HitZone, SCR_ParticleEmitter> m_mBleedingParticles;
	private SCR_CharacterHeadHitZone m_pHeadHitZone;
	private SCR_CharacterResilienceHitZone m_pResilienceHitZone;
	
	// TODO: Move these attributes to prefab data to save some memory
	[Attribute("", UIWidgets.Auto, desc: "Bleeding particle effect", category: "Bleeding", params: "ptc")]
	private ResourceName m_sBleedingParticle;
	
	[Attribute("0.5", UIWidgets.Auto, "Bleeding particle effect rate scale", category: "Bleeding")]
	private float m_fBleedingParticleRateScale;
	
	[Attribute("0.5", UIWidgets.Auto, desc: "Character bleeding rate multiplier", category: "Bleeding")]
	private float m_fBleedingRateScale;
	
	//-----------------------------------------------------------------------------------------------------------
	protected override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		
		UpdateBloodyFace();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	protected override void OnDamage(
				EDamageType type,
				float damage,
				HitZone pHitZone,
				IEntity instigator, 
				inout vector hitTransform[3], 
				float speed,
				int colliderID, 
				int nodeID)
	{
		super.OnDamage(type, damage, pHitZone, instigator, hitTransform, speed, colliderID, nodeID);
		
		UpdateBloodyFace();
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Invoked every time the DoT is added to certain hitzone.
	override void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType,  dps,  hz);
		
		if (hz.IsProxy())
			return;
		
		// Cancel regeneration when bleeding starts
		if (dType == EDamageType.REGENERATION)
			return;
		
		SCR_RegeneratingHitZone regenHZ = SCR_RegeneratingHitZone.Cast(hz);
		if (regenHZ)
			regenHZ.RemoveRegeneration();
	}

	//-----------------------------------------------------------------------------------------------------------
	//!	Invoked when provided damage type is removed from certain hitzone.
	override void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		super.OnDamageOverTimeRemoved(dType, hz);
		
		if (hz.IsProxy())
			return;
		
		// Schedule regeneration when bleeding stops
		if (dType == EDamageType.REGENERATION)
			return;
		
		SCR_RegeneratingHitZone regenHZ = SCR_RegeneratingHitZone.Cast(hz);
		if (regenHZ)
			regenHZ.ScheduleRegeneration();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateBloodClothes()
	{
		if (GetState() == EDamageState.DESTROYED)
		{
			GetGame().GetCallqueue().Remove(UpdateBloodClothes);
			return;
		}
		
		int bleedingHitZonesCount;
		if (m_aBleedingHitZones)
			bleedingHitZonesCount = m_aBleedingHitZones.Count();
		
		if (bleedingHitZonesCount == 0)
		{
			GetGame().GetCallqueue().Remove(UpdateBloodClothes);
			return;
		}
		
		SCR_CharacterHitZone characterHitZone;
		for (int i; i < bleedingHitZonesCount; i++)
		{
			characterHitZone = SCR_CharacterHitZone.Cast(m_aBleedingHitZones[i]);
			if (characterHitZone)
				characterHitZone.AddBloodToClothes();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateBloodyFace()
	{
		if (!m_pHeadHitZone || GetState() == EDamageState.DESTROYED)
		{
			GetGame().GetCallqueue().Remove(UpdateBloodyFace);
			return;
		}
		
		m_pHeadHitZone.SetWoundedSubmesh(ShouldHaveBloodyFace());
	}
	
	//-----------------------------------------------------------------------------------------------------------
	// Check if dmg threshold of critical is greater than the one of our current dmg state (This means we are in critical damage state or past critical)
	// result is true if any of the 3 virtual hitzones have enough damage to have entered/surpassed the defined damage state OR is bleeding
	private bool ShouldHaveBloodyFace()
	{
		if (m_aBleedingHitZones)
			return true;
		
		ECharacterDamageState headState = m_pHeadHitZone.GetDamageState();
		if (m_pHeadHitZone.GetDamageStateThreshold(headState) <= m_pHeadHitZone.GetDamageStateThreshold(ECharacterDamageState.WOUNDED))
			return true;
		
		HitZone healthHitZone = GetDefaultHitZone();
		if (healthHitZone.GetDamageStateThreshold(healthHitZone.GetDamageState()) <= healthHitZone.GetDamageStateThreshold(ECharacterHealthState.CRITICAL))
			return true;
		
		if (m_pResilienceHitZone)
		{
			if (m_pResilienceHitZone.GetDamageStateThreshold(m_pResilienceHitZone.GetDamageState()) <= m_pResilienceHitZone.GetDamageStateThreshold(ECharacterHealthState.CRITICAL))
				return true;
		}
		
		if (m_pBloodHitZone)
		{
			if (m_pBloodHitZone.GetDamageStateThreshold(m_pBloodHitZone.GetDamageState()) <= m_pBloodHitZone.GetDamageStateThreshold(ECharacterBloodState.FAINTING))
				return true;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetBloodHitZone(HitZone hitzone)
	{
		m_pBloodHitZone = SCR_CharacterBloodHitZone.Cast(hitzone);
	}

	//-----------------------------------------------------------------------------------------------------------
	HitZone GetBloodHitZone()
	{
		return m_pBloodHitZone;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void GetBleedingHitZones(out notnull array<HitZone> hitZones)
	{
		if (m_aBleedingHitZones)
			hitZones.Copy(m_aBleedingHitZones);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	float GetBleedingRateScale()
	{
		return m_fBleedingRateScale;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetHeadHitZone(HitZone hitzone)
	{
		m_pHeadHitZone = SCR_CharacterHeadHitZone.Cast(hitzone);
	}

	//-----------------------------------------------------------------------------------------------------------
	HitZone GetHeadHitZone()
	{
		return m_pHeadHitZone;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetResilienceHitZone(HitZone hitzone)
	{
		m_pResilienceHitZone = SCR_CharacterResilienceHitZone.Cast(hitzone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	HitZone GetResilienceHitZone()
	{
		return m_pResilienceHitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Add immediate blood effect to clothes covering a hitzone
	\param hitZoneIndex Index of hitzone
	\param immediateBloodEffect Amount of blood effect strength increase
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_AddBloodToClothes(int hitZoneIndex, float immediateBloodEffect)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (hitZone)
			hitZone.AddBloodToClothes(immediateBloodEffect);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Add bleeding from one hitzone
	\param hitZone Hitzone to get bleeding rate from and add effect to
	\param bleedingRate Rate of bleeding damage per second
	\param colliderDescriptorIndex Collider descriptor index
	*/
	void AddBleedingHitZone(notnull SCR_CharacterHitZone hitZone, int colliderDescriptorIndex = -1)
	{
		if (!m_aBleedingHitZones)
			m_aBleedingHitZones = {};
		
		m_aBleedingHitZones.Insert(hitZone);
		
		float bleedingRate = hitZone.GetMaxBleedingRate();
		
		if (m_pHeadHitZone)
			GetGame().GetCallqueue().CallLater(UpdateBloodyFace, BLOODY_FACE_UPDATE_PERIOD, true);
		GetGame().GetCallqueue().CallLater(UpdateBloodClothes, BLOOD_CLOTHES_UPDATE_PERIOD, true);
		CreateBleedingParticleEffect(hitZone, bleedingRate, colliderDescriptorIndex);
		
		// The rest of the code is handled on authority only
		if (hitZone.IsProxy())
			return;
		
		// This could be replaced with a simple boolean or float instead of DOT
		hitZone.SetDamageOverTime(EDamageType.BLEEDING, 1e-5);
		
		if (m_pBloodHitZone)
		{
			float currentBleeding = m_pBloodHitZone.GetDamageOverTime(EDamageType.BLEEDING);
			m_pBloodHitZone.SetDamageOverTime(EDamageType.BLEEDING, currentBleeding + bleedingRate);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Remove bleeding from one hitzone
	\param hitZone HitZone to remove bleeding from
	*/
	void RemoveBleedingHitZone(notnull HitZone hitZone)
	{
		RemoveBleedingParticleEffect(hitZone);
		if (m_aBleedingHitZones)
		{
			m_aBleedingHitZones.RemoveItem(hitZone);
			if (m_aBleedingHitZones.IsEmpty())
				m_aBleedingHitZones = null;
		}
		UpdateBloodyFace();
		
		if (hitZone.IsProxy())
			return;
		
		hitZone.SetDamageOverTime(EDamageType.BLEEDING, 0);
		
		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
		if (characterHitZone && m_pBloodHitZone)
		{
			float currentBleeding = m_pBloodHitZone.GetDamageOverTime(EDamageType.BLEEDING);
			float hitZoneBleeding = characterHitZone.GetMaxBleedingRate();
			m_pBloodHitZone.SetDamageOverTime(EDamageType.BLEEDING, Math.Max(currentBleeding - hitZoneBleeding, 0));
		}
		
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		int hitZoneIndex = hitZones.Find(hitZone);
		if (hitZoneIndex >= 0)
			Rpc(RpcDo_RemoveBleedingHitZone, hitZoneIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Create bleeding particle effect for a hitzone
	\param hitZoneIndex Index of hitzone
	\param bleedingRate Bleeding rate
	\param colliderDescriptorIndex Collider descriptor index
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_AddBleedingHitZone(int hitZoneIndex, int colliderDescriptorIndex)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (hitZone)
			hitZone.AddBleedingToCollider(colliderDescriptorIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Remove bleeding particle effect from a hitzone
	\param hitZoneIndex Index of hitzone
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RemoveBleedingHitZone(int hitZoneIndex)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		HitZone hitZone = hitZones.Get(hitZoneIndex);
		if (hitZone)
			RemoveBleedingHitZone(hitZone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Called to start bleeding effect on a specificied bone
	void CreateBleedingParticleEffect(notnull HitZone hitZone, float bleedingRate, int colliderDescriptorIndex)
	{
		// Play Bleeding particle
		if (m_sBleedingParticle.IsEmpty())
			return;
		
		RemoveBleedingParticleEffect(hitZone);
		
		if (bleedingRate == 0 || m_fBleedingParticleRateScale == 0)
			return;
		
		// TODO: Blood traces on ground that should be left regardless of clothing, perhaps just delayed
		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
		if (characterHitZone.IsCovered())
			return;
		
		// Get bone node
		vector transform[4];
		int boneIndex;
		int boneNode;
		if (!hitZone.TryGetColliderDescription(GetOwner(), colliderDescriptorIndex, transform, boneIndex, boneNode))
			return;
		
		SCR_ParticleEmitter particleEmitter = SCR_ParticleAPI.PlayOnObjectPTC(GetOwner(), m_sBleedingParticle, vector.Zero, vector.Zero, boneNode);
		SCR_ParticleAPI.LerpAllEmitters(particleEmitter, bleedingRate * m_fBleedingParticleRateScale, EmitterParam.BIRTH_RATE);
		
		if (!m_mBleedingParticles)
			m_mBleedingParticles = new map<HitZone, SCR_ParticleEmitter>;
		
		m_mBleedingParticles.Insert(hitZone, particleEmitter);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Get particle effect on a specified hitzone
	\param hitZone Bleeding HitZone to get particle effect for
	*/
	void RemoveBleedingParticleEffect(HitZone hitZone)
	{
		if (!m_mBleedingParticles)
			return;
		
		SCR_ParticleEmitter particleEmitter = m_mBleedingParticles.Get(hitZone);
		if (particleEmitter)
		{
			particleEmitter.Pause();
			m_mBleedingParticles.Remove(hitZone);
		}
		
		if (m_mBleedingParticles.IsEmpty())
			m_mBleedingParticles = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Stop all bleeding
	void RemoveBleeding()
	{		
		if (m_aBleedingHitZones)
		{
			HitZone hitZone;
			for (int i = m_aBleedingHitZones.Count() - 1; i >= 0; i--)
			{
				hitZone = m_aBleedingHitZones.Get(i);
				if (hitZone)
					RemoveBleedingHitZone(hitZone);
			}
		}
		
		if (m_pBloodHitZone)
			m_pBloodHitZone.SetDamageOverTime(EDamageType.BLEEDING, 0);
		
		UpdateBloodyFace();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a particular physical hitzone
	void AddParticularBleeding(string hitZoneName = "Chest")
	{
		SCR_CharacterHitZone targetHitZone = SCR_CharacterHitZone.Cast( GetHitZoneByName(hitZoneName));
		
		if (!targetHitZone)
			return;
		
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
				
		int hitZoneIndex = hitZones.Find(targetHitZone);
		if (hitZoneIndex >= 0)
		{
			int colliderDescriptorIndex = Math.RandomInt(0, targetHitZone.GetNumColliderDescriptors() - 1);
			RpcDo_AddBleedingHitZone(hitZoneIndex, colliderDescriptorIndex);
			Rpc(RpcDo_AddBleedingHitZone, hitZoneIndex, colliderDescriptorIndex);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a random physical hitzone
	void AddRandomBleeding()
	{
		array<HitZone> hitZones = {};
		array<HitZone> validHitZones = {};
		GetPhysicalHitZones(hitZones);
		
		foreach (HitZone hitZone: hitZones)
		{
			// Is not already bleeding
			if (hitZone.GetDamageOverTime(EDamageType.BLEEDING) > 0)
				continue;
			
			//Is character hz
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (characterHitZone)
				validHitZones.Insert(characterHitZone);
		}
		
		if (validHitZones.IsEmpty())
			return;
		
		HitZone randomHitZone = validHitZones.GetRandomElement();
		
		hitZones.Clear();
		GetAllHitZones(hitZones);
		
		int hitZoneIndex = hitZones.Find(randomHitZone);
		if (hitZoneIndex >= 0)
		{
			int colliderDescriptorIndex = Math.RandomInt(0, randomHitZone.GetNumColliderDescriptors() - 1);
			RpcDo_AddBleedingHitZone(hitZoneIndex, colliderDescriptorIndex);
			Rpc(RpcDo_AddBleedingHitZone, hitZoneIndex, colliderDescriptorIndex);
		}
	}

	//-----------------------------------------------------------------------------------------------------------
	void ~SCR_CharacterDamageManagerComponent()
	{
		RemoveBleeding();
	}
};
