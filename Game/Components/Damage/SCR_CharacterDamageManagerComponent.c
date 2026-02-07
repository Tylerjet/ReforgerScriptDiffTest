enum ECharacterHitZoneGroup : EHitZoneGroup
{
	HEAD = 10,
	UPPERTORSO = 20,
	LOWERTORSO = 30,
	LEFTARM = 40,
	RIGHTARM = 50,
	LEFTLEG = 60,
	RIGHTLEG = 70,
}

//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponentClass: SCR_DamageManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponent : SCR_DamageManagerComponent
{
	// 1000 ms timer for bloody clothes update
	static const int BLOOD_CLOTHES_UPDATE_PERIOD = 1000;
	
	// bleeding rate multiplier after death - used to stop particles sooner
	const float DEATH_BLEEDOUT_SCALE = 4;
	
	// Physics variables
	protected float m_fHighestContact;
	protected float m_fMinImpulse;

	// Static array for all limbs
	static ref array<ECharacterHitZoneGroup> LIMB_GROUPS;
	static const ref array<ECharacterHitZoneGroup> EXTREMITY_LIMB_GROUPS = {ECharacterHitZoneGroup.LEFTARM, ECharacterHitZoneGroup.RIGHTARM, ECharacterHitZoneGroup.LEFTLEG, ECharacterHitZoneGroup.RIGHTLEG};
	
	//replicated arrays for clients
	protected ref array<ECharacterHitZoneGroup> m_aTourniquettedGroups;
	protected ref array<ECharacterHitZoneGroup> m_aSalineBaggedGroups;
	protected ref array<int> m_aBeingHealedGroup;
	protected ref array<float> m_aGroupBleedingRates;
	
	protected ref map<SCR_CharacterHitZone, ref SCR_ArmoredClothItemData> m_mClothItemDataMap;
	protected SCR_CharacterBloodHitZone m_pBloodHitZone;
	protected ref array<HitZone> m_aBleedingHitZones;
	protected ref map<HitZone, ParticleEffectEntity> m_mBleedingParticles;
	protected SCR_CharacterResilienceHitZone m_pResilienceHitZone;
	protected SCR_CharacterHeadHitZone m_pHeadHitZone;

	// audio
	protected SCR_CommunicationSoundComponent m_CommunicationSound;

	static protected SCR_GameModeHealthSettings s_HealthSettings;
 	
	protected bool m_bDOTScaleChangedByGM;
	protected bool m_bRegenScaleChangedByGM;
	protected bool m_bUnconsciousnessSettingsChangedByGM;
	protected bool m_bOverrideCharacterMedical;
	
	// TODO: Move these attributes to prefab data to save some memory
	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourceNamePicker, desc: "Bleeding particle effect", params: "ptc", precision: 3, category: "Bleeding")]
	protected ResourceName m_sBleedingParticle;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Bleeding particle effect rate scale", params: "0 5 0.001", precision: 3, category: "Bleeding Mode")]
	protected float m_fBleedingParticleRateScale;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001", precision: 3, category: "Bleeding")]
	protected float m_fDOTScale;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001", precision: 3, category: "Regeneration")]
	protected float m_fRegenScale;
	
	[Attribute("0.3", UIWidgets.Auto, "Resilience regeneration scale while unconscious\n[x * 100%]")]
	protected float m_fUnconsciousRegenerationScale;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed", category: "Unconsciousness")]
	protected bool m_bPermitUnconsciousness;
	
	[Attribute(defvalue: "0.05", uiwidget: UIWidgets.Slider, desc: "Affects how much the bleeding is reduced", params: "0 1 0.001", precision: 3, category: "Tourniquets")]
	protected float m_fTourniquetStrengthMultiplier;
	
	//-----------------------------------------------------------------------------------------------------------
	event override void OnInit(IEntity owner)
	{
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (baseGameMode)
			s_HealthSettings = baseGameMode.GetGameModeHealthSettings();
		
		LIMB_GROUPS = {};
		SCR_Enum.GetEnumValues(ECharacterHitZoneGroup, LIMB_GROUPS);
		
#ifdef ENABLE_DIAG
		DiagInit(owner);
#endif
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Check whether character health state meets requirements for consciousness
	protected bool ShouldBeUnconscious()
	{
		HitZone bloodHZ = GetBloodHitZone();
		if (!bloodHZ)
			return false;
		
		ECharacterBloodState bloodState = bloodHZ.GetDamageState();
		if (bloodHZ.GetDamageStateThreshold(bloodState) <= bloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))	
			return true;
				
		HitZone resilienceHZ = GetResilienceHitZone();
		if (!resilienceHZ)
			return false;
		
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		ECharacterResilienceState resilienceState = resilienceHZ.GetDamageState();

		if (controller.IsUnconscious())
		{		
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED))
				return true;
		}
		else
		{
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.UNCONSCIOUS))
				return true;
		}
			
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateConsciousness()
	{
		bool unconscious = ShouldBeUnconscious();
		
		// If unconsciousness is not allowed, kill character
		// Also kill the character if the blood state is not high enough for being unconsciousness
		if (unconscious && (!GetPermitUnconsciousness() || (m_pBloodHitZone && m_pBloodHitZone.GetDamageState() == ECharacterBloodState.DESTROYED)))
		{
			Kill(GetInstigator());
			return;
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		controller.SetUnconscious(unconscious);
		}
	
	//------------------------------------------------------------------------------------------------
	//! Force unconsciousness regardless of health
	void ForceUnconsciousness(float resilienceHealth = 0)
	{
		if (!GetPermitUnconsciousness())
			return;
		
		HitZone resilienceHZ = GetResilienceHitZone();
		if (!resilienceHZ)
			return;
		
		resilienceHZ.SetHealth(resilienceHealth);
		UpdateConsciousness();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Insert armor data from clothing item prefab into map, create map if null
	void InsertArmorData(notnull SCR_CharacterHitZone charHitZone, SCR_ArmoredClothItemData attributes)
	{
		if (!m_mClothItemDataMap)
			m_mClothItemDataMap = new map<SCR_CharacterHitZone, ref SCR_ArmoredClothItemData>();
		
		m_mClothItemDataMap.Insert(charHitZone, attributes);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Remove armor data from clothing item prefab from map, delete map if empty
	void RemoveArmorData(notnull SCR_CharacterHitZone charHitZone)
	{
		if (!m_mClothItemDataMap)
			return;
		
		if (m_mClothItemDataMap.Contains(charHitZone))
			m_mClothItemDataMap.Remove(charHitZone);
		
		if (m_mClothItemDataMap.IsEmpty())
			m_mClothItemDataMap = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! If !remove, take data from prefab and insert to map as class. If remove, remove this hitZone's stored data from map
	void UpdateArmorDataMap(notnull SCR_ArmoredClothItemData armorAttr, bool remove)
	{
		foreach (string hitZoneName : armorAttr.m_aProtectedHitZones)
		{
			SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(GetHitZoneByName(hitZoneName));
			if (hitZone)
			{
				if (remove)
				{
					RemoveArmorData(hitZone);
					continue;
				}
				
				SCR_ArmoredClothItemData armoredClothItemData;
				armoredClothItemData = new SCR_ArmoredClothItemData(
					armorAttr.m_eProtection,
					armorAttr.m_aProtectedHitZones,
					armorAttr.m_aProtectedDamageTypes,
					armorAttr.m_MaterialResourceName
				);

				InsertArmorData(hitZone, armoredClothItemData);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override GameMaterial OverrideHitMaterial(HitZone struckHitzone)
	{
		SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(struckHitzone);
		if (!charHitZone)
			return null;
		
		SCR_ArmoredClothItemData armorData = GetArmorData(charHitZone);
		if (!armorData)
			return null;
		
		if (!armorData.m_Material)
			return null;
		
		return armorData.m_Material.GetResource().ToBaseContainer();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Get data from m_mClothItemDataMap
	SCR_ArmoredClothItemData GetArmorData(notnull SCR_CharacterHitZone charHitZone)
	{
		if (!m_mClothItemDataMap)
			return null;
		
		if (m_mClothItemDataMap.Contains(charHitZone))
			return m_mClothItemDataMap.Get(charHitZone);
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Get protection value of particular hitZone stored on m_mClothItemDataMap
	float GetArmorProtection(notnull SCR_CharacterHitZone charHitZone, EDamageType damageType)
	{
		SCR_ArmoredClothItemData localArmorData = GetArmorData(charHitZone);
		if (!localArmorData)
			return 0;

		foreach(EDamageType protectedType : localArmorData.m_aProtectedDamageTypes)
		{
			if (damageType == protectedType)
				return localArmorData.m_eProtection;
		}
		
		return 0;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Function called from SCR_ArmorDamageManagerComponent in case player is shot in armor. Applies effects
	void ArmorHitEventEffects(float damage)
	{
		SoundHit(true, EDamageType.KINETIC);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		CharacterInputContext context = controller.GetInputContext();

		if (context)
			context.SetHit(EHitReactionType.HIT_REACTION_LIGHT, 0);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Function called from SCR_ArmorDamageManagerComponent in case player is shot in armor. Applies damage
	void ArmorHitEventDamage(EDamageType type, float damage, IEntity instigator)
	{
		if (m_pResilienceHitZone)
			m_pResilienceHitZone.HandleDamage(damage, type, instigator);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Invoked every time the DoT is added to certain hitZone.
	override void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType,  dps,  hz);

		if (hz.IsProxy())
			return;
		
		// Play heal sound and cancel regeneration when bleeding starts
		if (dType == EDamageType.HEALING)
		{
			SoundHeal();
			return;
		}

		// Cancel regeneration when bleeding starts
		if (dType == EDamageType.REGENERATION)
			return;

		SCR_RegeneratingHitZone regenHZ = SCR_RegeneratingHitZone.Cast(hz);
		if (regenHZ)
			regenHZ.RemovePassiveRegeneration();
	}

	//-----------------------------------------------------------------------------------------------------------
	//!	Invoked when provided damage type is removed from certain hitZone.
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
	//!	Invoked when the default hitZone changes damagestate
	override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		
		if (state == EDamageState.DESTROYED)
		{
			if (IsRplReady() || !GetDefaultHitZone().IsProxy())
				SoundDeath();

			RemoveAllBleedingParticlesAfterDeath();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SoundHit(bool critical, EDamageType damageType)
	{
		// Ignore if knocked out or dead
		if (LocalGetLifeState() != ECharacterLifeState.ALIVE)
			return;

		if (m_CommunicationSound)
			m_CommunicationSound.SoundEventHit(critical, damageType);
	}

	//------------------------------------------------------------------------------------------------
	//! Tell m_CommunicationSound to stop character from screaming, otherwise ignore
	void SoundKnockout()
	{
		if (m_CommunicationSound && m_CommunicationSound.IsPlaying())
			m_CommunicationSound.SoundEventDeath(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Tell m_CommunicationSound that this character is now dead
	void SoundDeath()
	{
		bool silent;

		// Silent if killed with headshot
		if (m_pHeadHitZone && m_pHeadHitZone.GetDamageState() == EDamageState.DESTROYED)
			silent = true;

		// Silent if already knocked out
		if (LocalGetLifeState() == ECharacterLifeState.INCAPACITATED)
			silent = true;

		if (m_CommunicationSound)
			m_CommunicationSound.SoundEventDeath(silent);
	}

	//------------------------------------------------------------------------------------------------
	void SoundHeal()
	{
		if (m_CommunicationSound && LocalGetLifeState() == ECharacterLifeState.ALIVE)
			m_CommunicationSound.DelayedSoundEventPriority(SCR_SoundEvent.SOUND_VOICE_PAIN_RELIEVE, SCR_ECommunicationSoundEventPriority.SOUND_PAIN_RELIEVE, SCR_CommunicationSoundComponent.DEFAULT_EVENT_PRIORITY_DELAY);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//!	Clean up blood particle sources after death
	void RemoveAllBleedingParticlesAfterDeath()
	{
		float bleedingRate;
		if (m_pBloodHitZone)
			bleedingRate = DEATH_BLEEDOUT_SCALE * m_pBloodHitZone.GetDamageOverTime(EDamageType.BLEEDING);

		float delay;
		if (bleedingRate > 0)
			delay = m_pBloodHitZone.GetHealth() / bleedingRate;

		// Damage over time is not simulated after character death
		GetGame().GetCallqueue().Remove(RemoveAllBleedingParticles);
		GetGame().GetCallqueue().CallLater(RemoveAllBleedingParticles, delay * 1000);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void FullHeal(bool ignoreHealingDOT = true)
	{
		RemoveAllBleedings();
		super.FullHeal(ignoreHealingDOT);
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
	void SetBloodHitZone(HitZone hitZone)
	{
		m_pBloodHitZone = SCR_CharacterBloodHitZone.Cast(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	HitZone GetBloodHitZone()
	{
		return m_pBloodHitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	// Scale for effects that become worsened with blood level  
	float GetResilienceRegenScale()
	{
		if (!m_pBloodHitZone)
			return 1;

		float criticalBloodLevel = m_pBloodHitZone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);
		if (criticalBloodLevel >= 1)
			return 0;

		float currentBloodLevel = m_pBloodHitZone.GetDamageStateThreshold(m_pBloodHitZone.GetDamageState());
		if (currentBloodLevel < criticalBloodLevel)
			return 0;

		float resilienceRegenScale = (currentBloodLevel - criticalBloodLevel) / (1 - criticalBloodLevel);
		
		if (IsDamagedOverTime(EDamageType.BLEEDING) && LocalGetLifeState() == ECharacterLifeState.INCAPACITATED)
			resilienceRegenScale *= m_fUnconsciousRegenerationScale;

		return resilienceRegenScale;
	}

	//-----------------------------------------------------------------------------------------------------------
	void GetBleedingHitZones(out notnull array<HitZone> hitZones)
	{
		if (m_aBleedingHitZones)
			hitZones.Copy(m_aBleedingHitZones);
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetDOTScale(float rate, bool changed)
	{
		m_fDOTScale = rate;
		m_bDOTScaleChangedByGM = changed;
	}
	
	//------------------------------------------------------------------------------------------------
 	float GetDOTScale()
 	{
		if (!s_HealthSettings || m_bDOTScaleChangedByGM)
			return m_fDOTScale;
		
		return s_HealthSettings.GetBleedingScale();
 	}		
 	
 	//-----------------------------------------------------------------------------------------------------------
	void SetRegenScale(float rate, bool changed)
	{
		m_fRegenScale = rate;
		m_bRegenScaleChangedByGM = changed;
	}
	
	//------------------------------------------------------------------------------------------------
	// Return base regen scale
 	float GetRegenScale()
 	{
		if (!s_HealthSettings || m_bRegenScaleChangedByGM)
			return m_fRegenScale;
		
		return s_HealthSettings.GetRegenScale();
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether unconsciousness is permitted. Character prefab has highest authority, then gamemaster, then gamemode configuration
 	bool GetPermitUnconsciousness()
 	{
		if (!m_bPermitUnconsciousness)
			return false;
		
		if (!s_HealthSettings || m_bUnconsciousnessSettingsChangedByGM)
			return m_bPermitUnconsciousness;
		
		return s_HealthSettings.IsUnconsciousnessPermitted();
 	}	
 	
	//-----------------------------------------------------------------------------------------------------------
	void SetPermitUnconsciousness(bool permit, bool changed)
 	{
 		m_bPermitUnconsciousness = permit;
		m_bUnconsciousnessSettingsChangedByGM = changed;
 	}

	//-----------------------------------------------------------------------------------------------------------
	protected ECharacterLifeState LocalGetLifeState()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return ECharacterLifeState.ALIVE;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return ECharacterLifeState.ALIVE;
	
	 	return controller.GetLifeState();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetOverrideCharacterMedical(bool permit)
	{
		m_bOverrideCharacterMedical = permit;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetOverrideCharacterMedical()
	{
			return m_bOverrideCharacterMedical;
	}
 
	//-----------------------------------------------------------------------------------------------------------
	void SetResilienceHitZone(HitZone hitZone)
	{
		m_pResilienceHitZone = SCR_CharacterResilienceHitZone.Cast(hitZone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	HitZone GetResilienceHitZone()
	{
		return m_pResilienceHitZone;
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetHeadHitZone(HitZone hitZone)
	{
		m_pHeadHitZone = SCR_CharacterHeadHitZone.Cast(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	HitZone GetHeadHitZone()
	{
		return m_pHeadHitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Add immediate blood effect to clothes covering a hitZone
	\param hitZoneIndex Index of hitZone
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
	/*! Add bleeding from one hitZone
	\param hitZone Hitzone to get bleeding rate from and add effect to
	\param bleedingRate Rate of bleeding damage per second
	\param colliderDescriptorIndex Collider descriptor index
	*/
   	void AddBleedingHitZone(notnull SCR_CharacterHitZone hitZone, int colliderDescriptorIndex = -1)
	{
		if (!m_aBleedingHitZones)
			m_aBleedingHitZones = {};

		if (!m_aBleedingHitZones.Contains(hitZone))
			m_aBleedingHitZones.Insert(hitZone);
		
		// In case bleeding is started outside of normal context, full health will prevent DOT. This block will circumvent this issue
		float hitZoneDamageMultiplier = hitZone.GetHealthScaled();
		float bleedingRate = hitZone.GetMaxBleedingRate() - hitZone.GetMaxBleedingRate() * hitZoneDamageMultiplier;

		GetGame().GetCallqueue().CallLater(UpdateBloodClothes, BLOOD_CLOTHES_UPDATE_PERIOD, true);
		
		if (colliderDescriptorIndex == -1)
			colliderDescriptorIndex = Math.RandomInt(0, hitZone.GetNumColliderDescriptors() - 1);
		
		CreateBleedingParticleEffect(hitZone, bleedingRate, colliderDescriptorIndex);

		if (m_pBloodHitZone)
		{
			SCR_BleedingHitZoneParameters localBleedingHZParams = new SCR_BleedingHitZoneParameters(hitZone, bleedingRate);
			m_pBloodHitZone.AddBleedingHZToMap(hitZone, localBleedingHZParams);
			UpdateBleedingHitZones();
		}

		// The rest of the code is handled on authority only
		if (hitZone.IsProxy())
			return;

		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		int hitZoneIndex = hitZones.Find(hitZone);
		if (hitZoneIndex >= 0)
			Rpc(RpcDo_AddBleedingHitZone, hitZoneIndex, colliderDescriptorIndex, GetGroupBleedingRate(hitZone.GetHitZoneGroup()));
	}

	//-----------------------------------------------------------------------------------------------------------
	/*! Remove bleeding from one hitZone
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

		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
		if (characterHitZone && m_pBloodHitZone)
			m_pBloodHitZone.RemoveBleedingHZFromMap(characterHitZone);

		if (hitZone.IsProxy())
			return;
		
		hitZone.SetDamageOverTime(EDamageType.BLEEDING, 0);
		UpdateBleedingHitZones();
		
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		int hitZoneIndex = hitZones.Find(hitZone);
		if (hitZoneIndex >= 0 && !GetOwner().IsDeleted())
			Rpc(RpcDo_RemoveBleedingHitZone, hitZoneIndex, GetGroupBleedingRate(characterHitZone.GetHitZoneGroup()));
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBleedingHitZones()
	{
		SCR_CharacterHitZone hitZone;
		float bleedingDamage;
		float totalBleedingDamage;
		
		if (!m_pBloodHitZone.GetHitZoneDOTMap())
		{
			m_pBloodHitZone.SetDamageOverTime(EDamageType.BLEEDING, 0);
			m_aGroupBleedingRates = null;
			return;
		}
		
		m_aGroupBleedingRates = {};
		foreach(int enumValue : LIMB_GROUPS)
		{
			m_aGroupBleedingRates.Insert(0);
		}

		foreach (SCR_BleedingHitZoneParameters localBleedingHZParams : m_pBloodHitZone.GetHitZoneDOTMap())
		{
			if (!localBleedingHZParams)
				continue;
			
			hitZone = SCR_CharacterHitZone.Cast(localBleedingHZParams.m_hHitZone);
			ECharacterHitZoneGroup hitZoneGroup = hitZone.GetHitZoneGroup();
			
			if (localBleedingHZParams.m_fDamageOverTime != 0)
				hitZone.SetDamageOverTime(EDamageType.BLEEDING, 1e-5);
			
			if (!GetGroupTourniquetted(hitZoneGroup))
				bleedingDamage = localBleedingHZParams.m_fDamageOverTime;
			else
				bleedingDamage = localBleedingHZParams.m_fDamageOverTime * m_fTourniquetStrengthMultiplier;
			
			int hitZoneGroupIndex = LIMB_GROUPS.Find(hitZoneGroup);
			m_aGroupBleedingRates[hitZoneGroupIndex] = m_aGroupBleedingRates[hitZoneGroupIndex] + localBleedingHZParams.m_fDamageOverTime;
			
			totalBleedingDamage += bleedingDamage;
		}
		
		m_pBloodHitZone.SetDamageOverTime(EDamageType.BLEEDING, totalBleedingDamage);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Create bleeding particle effect for a hitZone
	\param hitZoneIndex Index of hitZone
	\param colliderDescriptorIndex Collider descriptor index
	\param bleedingRate Bleeding rate
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_AddBleedingHitZone(int hitZoneIndex, int colliderDescriptorIndex, float groupBleedingRate)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (!hitZone)
			return;
		
		AddBleedingHitZone(hitZone, colliderDescriptorIndex);
	
		if (groupBleedingRate >= 0)
			SetGroupBleedingRate(hitZone.GetHitZoneGroup(), groupBleedingRate);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Remove bleeding particle effect from a hitZone
	\param hitZoneIndex Index of hitZone
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RemoveBleedingHitZone(int hitZoneIndex, float groupBleedingRate)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (!hitZone)
			return;

		RemoveBleedingHitZone(hitZone);
		
		if (groupBleedingRate >= 0)
			SetGroupBleedingRate(hitZone.GetHitZoneGroup(), groupBleedingRate);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Called to start bleeding effect on a specificied bone
	void CreateBleedingParticleEffect(notnull HitZone hitZone, float bleedingRate, int colliderDescriptorIndex)
	{
		if (System.IsConsoleApp())
			return;
		
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

		// Create particle emitter
		ParticleEffectEntitySpawnParams spawnParams();
		spawnParams.Parent = GetOwner();
		spawnParams.PivotID = boneNode;
		ParticleEffectEntity particleEmitter = ParticleEffectEntity.SpawnParticleEffect(m_sBleedingParticle, spawnParams);
		if (System.IsConsoleApp())
			return;		
		
		if (!particleEmitter)
		{
			Print("Particle emitter: " + particleEmitter.ToString() + " There was a problem with creating the particle emitter: " + m_sBleedingParticle, LogLevel.WARNING);
			return;
		}

		// Track particle emitter in array
		if (!m_mBleedingParticles)
			m_mBleedingParticles = new map<HitZone, ParticleEffectEntity>;

		m_mBleedingParticles.Insert(hitZone, particleEmitter);

		// Play particles
		Particles particles = particleEmitter.GetParticles();
		if (particles)
			particles.MultParam(-1, EmitterParam.BIRTH_RATE, bleedingRate * m_fBleedingParticleRateScale);
		else
			Print("Particle: " + particles.ToString() + " Bleeding particle likely not created properly: " + m_sBleedingParticle, LogLevel.WARNING);
	}

	//-----------------------------------------------------------------------------------------------------------
	/*! Get particle effect on a specified hitZone
	\param hitZone Bleeding HitZone to get particle effect for
	*/
	void RemoveBleedingParticleEffect(HitZone hitZone)
	{
		if (!m_mBleedingParticles)
			return;

		ParticleEffectEntity particleEmitter = m_mBleedingParticles.Get(hitZone);
		if (particleEmitter)
		{
			particleEmitter.StopEmission();
			m_mBleedingParticles.Remove(hitZone);
		}

		if (m_mBleedingParticles.IsEmpty())
			m_mBleedingParticles = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Remove all bleeding particle effects from all bleeding hitZones
	void RemoveAllBleedingParticles()
	{
		if (!m_aBleedingHitZones || m_aBleedingHitZones.IsEmpty())
			return;
		
		foreach (HitZone hitZone : m_aBleedingHitZones)
			RemoveBleedingParticleEffect(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Stop all bleeding
	void RemoveAllBleedings()
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
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Stop bleeding from particular hitZone group
	void RemoveGroupBleeding(ECharacterHitZoneGroup charHZGroup)
	{
		if (!m_aBleedingHitZones)
			return;
		
		if (charHZGroup == ECharacterHitZoneGroup.VIRTUAL)
			return;
		
		array<HitZone> aBleedingHitZones = {};
		aBleedingHitZones.Copy(m_aBleedingHitZones);
		SCR_CharacterHitZone charHitZone;
		foreach (HitZone hitZone : aBleedingHitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);				
			if (!charHitZone || charHitZone.GetHitZoneGroup() != charHZGroup)
				continue;

			RemoveBleedingHitZone(hitZone);
		}
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a particular physical hitZone
	void AddParticularBleeding(string hitZoneName = "Chest", ECharacterDamageState intensityEnum = ECharacterDamageState.WOUNDED, float intensityFloat = -1)
	{
		SCR_CharacterHitZone targetHitZone = SCR_CharacterHitZone.Cast( GetHitZoneByName(hitZoneName));

		if (!targetHitZone || targetHitZone.IsProxy())
			return;
		
		// If someone wants to add bleeding to undamaged hitZone, return
		if (intensityEnum == ECharacterDamageState.UNDAMAGED)
			return;
		
		// if intensityFloat is unset use the enum instead
		if (intensityFloat < 0)
			targetHitZone.SetHealthScaled(targetHitZone.GetDamageStateThreshold(ECharacterDamageState.WOUNDED));
		else
			targetHitZone.SetHealthScaled(intensityFloat);

		AddBleedingHitZone(targetHitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a random physical hitZone
	void AddRandomBleeding()
	{
	//	TODO@FAC figure out why AI don't heal themselves after this function is called on them
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

		
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(validHitZones.GetRandomElement());
		AddBleedingHitZone(hitZone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup FindAssociatedHitZoneGroup(EBandagingAnimationBodyParts bodyPartToBandage)
	{
		array<HitZone> hitZones = {};
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetBodyPartToHeal() == bodyPartToBandage)
				return charHitZone.GetHitZoneGroup();		
		}
				
		return null;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	EBandagingAnimationBodyParts FindAssociatedBandagingBodyPart(ECharacterHitZoneGroup hitZoneGroup)
	{
		array<HitZone> hitZones = {};
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetHitZoneGroup() == hitZoneGroup)
				return charHitZone.GetBodyPartToHeal();		
		}
				
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void GetBandageAnimHitzones(EBandagingAnimationBodyParts eBandagingAnimBodyParts, out notnull array<HitZone> GroupHitZones)
	{
		array<HitZone> allGroupedHitZones = {};
		GetAllHitZones(allGroupedHitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : allGroupedHitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetBodyPartToHeal() == eBandagingAnimBodyParts)
				GroupHitZones.Insert(hitZone);
		}
	}
		
	//-----------------------------------------------------------------------------------------------------------
	float GetGroupHealthScaled(ECharacterHitZoneGroup hitZoneGroup)
	{
		float totalGroupHealth;
		float hitZoneQuantity;
		
		array<HitZone> allGroupedHitZones = {};
		GetAllHitZones(allGroupedHitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : allGroupedHitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!charHitZone || charHitZone.GetHitZoneGroup() != hitZoneGroup)
				continue;
			
			totalGroupHealth += charHitZone.GetHealthScaled();	
			hitZoneQuantity ++;
		}
		
		if (hitZoneQuantity <= 0)
			return 0;
		
		return totalGroupHealth / hitZoneQuantity;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get group bleeding rate for specific hitZone group
	private float GetGroupBleedingRate(ECharacterHitZoneGroup group)
	{
		if (!m_aGroupBleedingRates)
			return 0;
		
		if (group < 0 && group >= m_aGroupBleedingRates.Count())
			return 0;
		
		return m_aGroupBleedingRates[LIMB_GROUPS.Find(group)];
	}
	
	//------------------------------------------------------------------------------------------------
	// Set bleeding rate
	/*!
	Update bleeding rates for all hitZone groups
	\param hitZoneIndex Index of hitZone
	\param colliderDescriptorIndex Collider descriptor index
	\param bleedingRate Bleeding rate
	*/
	private void SetGroupBleedingRate(ECharacterHitZoneGroup group, float rate)
	{
		if (!m_aGroupBleedingRates && rate > 0)
		{		
			m_aGroupBleedingRates = {};
			foreach(int enumValue : LIMB_GROUPS)
			{
				m_aGroupBleedingRates.Insert(0);
			}
		}
		
		//If trying to set bleeding rate to current bleeding rate, fail
		if (!m_aGroupBleedingRates || m_aGroupBleedingRates[LIMB_GROUPS.Find(group)] == rate)
			return;
		
		m_aGroupBleedingRates[LIMB_GROUPS.Find(group)] = rate;
		
		foreach (float bleedingRate: m_aGroupBleedingRates)
		{
			if (bleedingRate > 0)
				return;			
		}
		
		//If having failed to set any bleedingrates, set array back to null
		m_aGroupBleedingRates = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override float GetGroupDamageOverTime(ECharacterHitZoneGroup hitZoneGroup, EDamageType damageType)
	{
		if (damageType == EDamageType.BLEEDING)
			return GetGroupBleedingRate(hitZoneGroup);

		return super.GetGroupDamageOverTime(hitZoneGroup, damageType);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetGroupTourniquetted(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aTourniquettedGroups && m_aTourniquettedGroups.Contains(hitZoneGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTourniquettedGroup(ECharacterHitZoneGroup hitZoneGroup, bool setTourniquetted)
	{
		if (setTourniquetted)
		{
			if (!m_aTourniquettedGroups)
				m_aTourniquettedGroups = {};
			else if (m_aTourniquettedGroups.Contains(hitZoneGroup))
				return;
			
			m_aTourniquettedGroups.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aTourniquettedGroups || !m_aTourniquettedGroups.Contains(hitZoneGroup))
				return;
			else
				m_aTourniquettedGroups.RemoveItem(hitZoneGroup);
	
			if (m_aTourniquettedGroups.IsEmpty())
				m_aTourniquettedGroups = null;
		}
		
		UpdateBleedingHitZones();
		UpdateCharacterGroupDamage(hitZoneGroup);
	}

	//-----------------------------------------------------------------------------------------------------------
	bool GetGroupIsBeingHealed(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aBeingHealedGroup && m_aBeingHealedGroup.Contains(hitZoneGroup);
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupIsBeingHealed(ECharacterHitZoneGroup hitZoneGroup, bool setIsBeingHealed)
	{
		if (setIsBeingHealed)
		{
			if (!m_aBeingHealedGroup)
				m_aBeingHealedGroup = {};
			else if (m_aBeingHealedGroup.Contains(hitZoneGroup))
				return;

			m_aBeingHealedGroup.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aBeingHealedGroup || !m_aBeingHealedGroup.Contains(hitZoneGroup))
				return;
			else
				m_aBeingHealedGroup.RemoveItem(hitZoneGroup);

			if (m_aBeingHealedGroup.IsEmpty())
				m_aBeingHealedGroup = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	bool GetGroupSalineBagged(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aSalineBaggedGroups && m_aSalineBaggedGroups.Contains(hitZoneGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hitZoneGroup
	//! \param[in] setSalineBagged
	void SetSalineBaggedGroup(ECharacterHitZoneGroup hitZoneGroup, bool setSalineBagged)
	{
		if (setSalineBagged)
		{
			if (!m_aSalineBaggedGroups)
				m_aSalineBaggedGroups = {};
			else if (m_aSalineBaggedGroups.Contains(hitZoneGroup))
				return;

			m_aSalineBaggedGroups.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aSalineBaggedGroups || !m_aSalineBaggedGroups.Contains(hitZoneGroup))
				return;
			else
				m_aSalineBaggedGroups.RemoveItem(hitZoneGroup);

			if (m_aSalineBaggedGroups.IsEmpty())
				m_aSalineBaggedGroups = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	HitZone GetMostDOTHitZone(EDamageType damageType, bool includeVirtualHZs = false, array<EHitZoneGroup> allowedGroups = null)
	{
		float highestDOT;
		float localDOT;
		HitZone highestDOTHitZone;
		array<HitZone> hitZones = {};
		
		if (includeVirtualHZs)
			GetAllHitZones(hitZones);
		else
			GetPhysicalHitZones(hitZones);
		
		foreach (HitZone hitZone : hitZones)
		{
			if (allowedGroups)
			{
				SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(hitZone);
				if (!charHitZone)
					continue;
				
				if (!allowedGroups.Contains(charHitZone.GetHitZoneGroup()))
					continue;
			}
			
			localDOT = hitZone.GetDamageOverTime(damageType);
			if (localDOT > highestDOT)
			{
				highestDOT = localDOT;
				highestDOTHitZone = hitZone;
			}
		}
		return highestDOTHitZone;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	static void GetAllLimbs(notnull out array<ECharacterHitZoneGroup> limbs)
	{
		limbs.Copy(LIMB_GROUPS);
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	static void GetAllExtremities(notnull out array<ECharacterHitZoneGroup> limbs)
	{
		limbs.Copy(EXTREMITY_LIMB_GROUPS);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateCharacterGroupDamage(ECharacterHitZoneGroup hitZoneGroup)
	{
		float AIMING_DAMAGE_MULTIPLIER = 4;
		float TOURNIQUETTED_LEG_DAMAGE = 0.7;
		float TOURNIQUETTED_ARMS_DAMAGE = 0.7;
		// Updated OnDamageStateChanged, movement damage is equal to the accumulative damage of all hitZones in the limb divided by the amount of limbs of type
		if (hitZoneGroup == ECharacterHitZoneGroup.LEFTLEG || hitZoneGroup == ECharacterHitZoneGroup.RIGHTLEG)
		{
			float leftLegDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG))
				leftLegDamage = TOURNIQUETTED_LEG_DAMAGE;
			else
				leftLegDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.LEFTLEG));
			
			float rightLegDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.RIGHTLEG))
				rightLegDamage = TOURNIQUETTED_LEG_DAMAGE;			
			else
				rightLegDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.RIGHTLEG));

			float legDamage = (leftLegDamage + rightLegDamage) * 0.5;
			SetMovementDamage( legDamage );
		}

		if (hitZoneGroup == ECharacterHitZoneGroup.LEFTARM || hitZoneGroup == ECharacterHitZoneGroup.RIGHTARM)
		{
			float leftArmDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTARM))
				leftArmDamage = TOURNIQUETTED_ARMS_DAMAGE;			
			else
				leftArmDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.LEFTARM));

			float rightArmDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.RIGHTARM))
				rightArmDamage = TOURNIQUETTED_ARMS_DAMAGE;
			else
				rightArmDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.RIGHTARM));

			float armDamage = (leftArmDamage + rightArmDamage) * 0.5;
			SetAimingDamage(armDamage * AIMING_DAMAGE_MULTIPLIER);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Determine which hitZones group is taking highest DOT
	\param damageTypes Which damagetype will be checked on the hitZones for highest damage amount
	\param onlyExtremities Only compare the 4 extremities (both arms, both legs)
	\param ignoreTQdHitZones Ignore tourniquetted limbs when looking for the one most bleeding one
	*/
	ECharacterHitZoneGroup GetCharMostDOTHitzoneGroup(EDamageType damageType, bool onlyExtremities = false, bool ignoreTQdHitZones = false, bool ignoreIfBeingTreated = false)
	{
		if (!m_aBleedingHitZones)
			return null;

		array<float> DOTValues = {};
		typename groupEnum = ECharacterHitZoneGroup;
		int groupCount = groupEnum.GetVariableCount();

		for (int i; i < groupCount; i++)
			DOTValues.Insert(0);

		float highestDOT;
		float localDOT;
		array<HitZone> hitZones = {}; 
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		ECharacterHitZoneGroup group;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!charHitZone)
				continue;
		
			// Virtual hitZones don't count to the accumulation of bleedings
			group = charHitZone.GetHitZoneGroup();
			if (group == EHitZoneGroup.VIRTUAL)
				continue;
			
			// If only extremities are desired for checking, continue if group is not an extremity
			if (onlyExtremities && !EXTREMITY_LIMB_GROUPS.Contains(group))
				continue;

			float DOT = m_pBloodHitZone.GetPhysicalHZBleedingDOT(charHitZone);
			if (DOT == 0)
				continue;
			
			// GetPhysicalHZBleedingDOT() is unaware of tourniquet status, so it is reduced seperately here.
			if (GetGroupTourniquetted(group))
			{
				DOT *= m_fTourniquetStrengthMultiplier;
				if (ignoreTQdHitZones)
				{
					DOTValues[LIMB_GROUPS.Find(group)] = 0;
					continue;
				}
			}
			
			// if desired, bleedingHitzones that are being treated are skipped so another hitZone will be healed by this inquiry
			if (GetGroupIsBeingHealed(group))
			{
				if (ignoreIfBeingTreated)
				{
					DOTValues[LIMB_GROUPS.Find(group)] = 0;
					continue;
				}
			}
			
			DOTValues[LIMB_GROUPS.Find(group)] = DOTValues[LIMB_GROUPS.Find(group)] + DOT;
		}
		
		ECharacterHitZoneGroup mostDOTHitZoneGroup;
		
		for (int i; i < groupCount; i++)
		{
			if (DOTValues[i] > highestDOT)
			{
				highestDOT = DOTValues[i];
				mostDOTHitZoneGroup = i;
			}
		}
		
		return LIMB_GROUPS.Get(mostDOTHitZoneGroup);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Get bone name for given hz group
	string GetBoneName(ECharacterHitZoneGroup hzGroup)
	{
		switch (hzGroup)
		{
			case ECharacterHitZoneGroup.HEAD:
			{
				return "Head";
			} break;

			case ECharacterHitZoneGroup.UPPERTORSO:
			{
				return "Spine5";
			} break;

			case ECharacterHitZoneGroup.LOWERTORSO:
			{
				return "Spine1";
			} break;

			case ECharacterHitZoneGroup.LEFTARM:
			{
				return "LeftForeArm";
			} break;

			case ECharacterHitZoneGroup.RIGHTARM:
			{
				return "RightForeArm";
			} break;

			case ECharacterHitZoneGroup.LEFTLEG:
			{
				return "LeftKnee";
			} break;

			case ECharacterHitZoneGroup.RIGHTLEG:
			{
				return "RightKnee";
			}
		}

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_CommunicationSound = SCR_CommunicationSoundComponent.Cast(owner.FindComponent(SCR_CommunicationSoundComponent));
		
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || !rpl.IsProxy())
		{
			Physics physics = owner.GetPhysics();
			// CallLater because GetOwner().GetPhysics() returns empty until GC finishes onPostInit
			GetGame().GetCallqueue().CallLater(SetExactMinImpulse, param1:owner);
		}
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE,"","Log player damage","GameCode");
		
		if (System.IsCLIParam("logPlayerDamage"))
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE, true);
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetExactMinImpulse(IEntity owner)
	{
		if (!owner)
			return;
		
		m_fMinImpulse = 80;
		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMinImpulse = physics.GetMass();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Character has EventMask.Contact disabled for performance reasons. This function is called instead from physicsobjects that do.
	//! \param owner Function is called with other as owner, so owner here is always same as GetOwner()
	//! \param other Other here is alawys the entity that collided with the character
	//! \param contact Contact data class passed by OnContact event from other
	void ContactDamage(IEntity owner, IEntity other, Contact contact)
	{
		if (Math.AbsFloat(contact.GetRelativeNormalVelocityBefore()) > 3)
		{
			ComputeCollisionDamage(owner, other, contact);
			
			if (LocalGetLifeState() == ECharacterLifeState.ALIVE)
				ForceUnconsciousness(0.3);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeCollisionDamage(notnull IEntity owner, notnull IEntity other, notnull Contact contact)
	{
		// Do not recompute collisiondamage within the impulseDelay if it's lower than the highest one thus far
		float relativeNormalVelocityBefore = Math.AbsFloat(contact.GetRelativeNormalVelocityBefore());
		if (m_fHighestContact > relativeNormalVelocityBefore)
			return;
		
		m_fHighestContact = relativeNormalVelocityBefore;
		
		float momentumCharacterThreshold = m_fMinImpulse * 1 * KM_PER_H_TO_M_PER_S;
		float momentumCharacterDestroy = m_fMinImpulse * 100 * KM_PER_H_TO_M_PER_S;
		float damageScaleToCharacter = (momentumCharacterDestroy - momentumCharacterThreshold) * 0.0001;
		
		float impactMomentum = Math.AbsFloat(m_fMinImpulse * relativeNormalVelocityBefore);

		float damageValue = damageScaleToCharacter * (impactMomentum - momentumCharacterThreshold);
		if (damageValue <= 0)
			return;
		
		// We apply the collisiondamage only every 200 ms at most. 
		// If a bigger collision happens within this time, this collisions Contact data will overwrite the previous collision, but does not reset the remaining time until it is applied.
		int impulseDelay = 200;
		int remainingTime = GetGame().GetCallqueue().GetRemainingTime(ApplyCollisionDamage);
		if (remainingTime == -1)
		{
			GetGame().GetCallqueue().CallLater(ApplyCollisionDamage, impulseDelay, false, contact, other, damageValue);
		}
		else 
		{
			GetGame().GetCallqueue().Remove(ApplyCollisionDamage);
			GetGame().GetCallqueue().CallLater(ApplyCollisionDamage, remainingTime, false, contact, other, damageValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyCollisionDamage(Contact contact, IEntity other, float damageValue)
	{
		if (!GetOwner())
			return;
		
		array<HitZone> characterHitZones = {};
		GetAllHitZones(characterHitZones);
		
		// Apply collisionDamage only to the 6 nearest hitzones to the contactpoint
		int hitZonesReturnAmount = 6;
		GetNearestHitZones(contact.Position, characterHitZones, hitZonesReturnAmount);
		
		foreach (HitZone characterHitZone : characterHitZones)
		{
			if (characterHitZone.GetDamageState() == EDamageState.DESTROYED)
				continue;

			characterHitZone.HandleDamage(damageValue / hitZonesReturnAmount, EDamageType.COLLISION, other);
		}
		
		m_fHighestContact = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get nearest hitZones to the point of impact
	//! \param worldPosition Position of the point of impact
	//! \param[inout] nearestHitZones Array of hitZones which colliders' positions will be compared with worldPosition. Returns the nearest hitZones to worldPosition in descending order of distance
	//! \param hitZonesReturnAmount Amount of hitZones that are returned in nearestHitZones. IF all are wanted, set nearestHitZones.Count().
	void GetNearestHitZones(vector worldPosition, notnull inout array<HitZone> nearestHitZones, int hitZonesReturnAmount)
	{
		array<vector> hitZonePositions  = {};
		array<int> IDs = {};
		foreach (HitZone hitZone : nearestHitZones)
		{
			if (!hitZone.HasColliderNodes())
				continue;
			
			IDs.Clear();
			hitZone.GetColliderIDs(IDs);
			foreach (int ID : IDs)
			{
				vector colliderPos[4];
				GetOwner().GetPhysics().GetGeomWorldTransform(ID, colliderPos);
				vector relativePosition = worldPosition.InvMultiply4(colliderPos);
				vector hitZonePosition = {relativePosition.Length(), ID, 0};
				hitZonePositions.Insert(hitZonePosition);
			}
		}

		// Order hitZonePositions by distance from close to furthest
		hitZonePositions.Sort();

		array<int> closestIDs = {};
		for (int i; i < hitZonesReturnAmount; i++)
		{
			vector hitZonePosition = hitZonePositions[i];
			closestIDs.InsertAt(hitZonePosition[1], i);
		}
		
		nearestHitZones.Clear();
		GetHitZonesByColliderIDs(nearestHitZones, closestIDs);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleFallDamage(float damage)
	{
		array<HitZone> targetHitZones = {};
		GetHitZonesOfGroup(ECharacterHitZoneGroup.LEFTLEG, targetHitZones, true);
		GetHitZonesOfGroup(ECharacterHitZoneGroup.RIGHTLEG, targetHitZones, false);
		
		float overDamageCutOff = 50;
		if (damage > overDamageCutOff)
			GetHitZonesOfGroup(ECharacterHitZoneGroup.LOWERTORSO, targetHitZones, false);
		
		damage *= 2;
		
		foreach (HitZone hitZone : targetHitZones)
			hitZone.HandleDamage(damage/targetHitZones.Count(), EDamageType.COLLISION, GetOwner());
	}

	#ifdef ENABLE_DIAG	
	//-----------------------------------------------------------------------------------------------------------
	protected override void OnDamage(notnull BaseDamageContext damageContext)
	{		
		super.OnDamage(damageContext);

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE))
		{
			SCR_HitZone scriptedHz = SCR_HitZone.Cast(damageContext.struckHitZone);
			if (!scriptedHz)
				return;

			IEntity hzOwner = scriptedHz.GetOwner();
			if (!hzOwner)
				return;
			
			string instigatorName;
			int instigatorID = damageContext.instigator.GetInstigatorPlayerID();
			IEntity instigatorEntity = damageContext.instigator.GetInstigatorEntity();
			
			if (instigatorID > 0)
			{
				instigatorName = GetGame().GetPlayerManager().GetPlayerName(instigatorID);
			}
			else
			{
				ResourceName prefabName;
				if (instigatorEntity)
				{
					EntityPrefabData prefabData = instigatorEntity.GetPrefabData();
					if (prefabData)
						prefabName = prefabData.GetPrefabName();
				}
				
				if (prefabName.IsEmpty())
				{
					if (instigatorEntity)
					{
						instigatorName = ((instigatorEntity.GetID()).ToString());
					}
					else
					{
						instigatorName = instigatorEntity.ToString();
					}
				}
				else
				{
					TStringArray strs = new TStringArray;
					prefabName.Split("/", strs, true);
					instigatorName = ((instigatorEntity.GetID()).ToString()) + strs[strs.Count() - 1];
				}
			}
			
			string hzOwnerName;
			int hzOwnerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(hzOwner);
			if (hzOwnerID > 0)
			{
				hzOwnerName = GetGame().GetPlayerManager().GetPlayerName(hzOwnerID);
			}
			else
			{
				EntityPrefabData prefabData = hzOwner.GetPrefabData();
				ResourceName prefabName = prefabData.GetPrefabName();
				
				if (prefabName.IsEmpty())
				{
					hzOwnerName = ((hzOwner.GetID()).ToString());
				}
				else
				{
					TStringArray strs = new TStringArray;
					prefabName.Split("/", strs, true);
					hzOwnerName = ((hzOwner.GetID()).ToString()) + strs[strs.Count() - 1];
				}
			}

			if (EntityUtils.IsPlayer(instigatorEntity) || EntityUtils.IsPlayer(hzOwner))
				PrintFormat("HIT LOG: (%1) damaged (%2) - [Damage = %3, Speed = %4]", instigatorName, hzOwnerName, damageContext.damageValue, damageContext.impactVelocity);
		}
	}

	override void OnDelete(IEntity owner)
	{
		DisconnectFromDiagSystem(owner);
		
		super.OnDelete(owner);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void DiagInit(IEntity owner)
	{
		ConnectToDiagSystem(owner);
		// Register to debug menu
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_DAMAGE,"","Deal damage debug","Character");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_DAMAGE,0);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDiag(IEntity owner, float timeSlice)
	{
		ProcessDebug(owner);
	}

	//------------------------------------------------------------------------------------------------
	void ProcessDebug(IEntity owner)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_DAMAGE))
		{
			// Only apply debug damages to currently controlled character
			IEntity playerEntity = GetGame().GetPlayerController().GetControlledEntity();
			if (playerEntity != owner)
				return;
			
			DbgUI.Text("See: GameCode >> Hit Zones >> Player HitZones for damages");
			DbgUI.Text("Applying damage from client is NOT possible");
			DbgUI.Text("Only conventional DOT damage are shown in debug");
			DbgUI.Text("Y to damage every hitZone for 4 damage");
			DbgUI.Text("U to reduce chest health by 10");
			DbgUI.Text("I to reduce right thigh health by 10");
			DbgUI.Text("O to reduce left thigh health by 10");
			DbgUI.Text("P to add bleeding to right thigh");
			DbgUI.Text("T to add bleeding to left thigh");
			DbgUI.Text("K to reset damage effect");
		
			vector hitPosDirNorm[3];
			
			if (Debug.KeyState(KeyCode.KC_Y))
			{
				Debug.ClearKey(KeyCode.KC_Y);
				AddParticularBleeding("RThigh", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("LThigh", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("Chest", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("Head", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("LArm", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("Abdomen", intensityFloat: Math.RandomFloat(0.9, 0.99));
				AddParticularBleeding("RArm", intensityFloat: Math.RandomFloat(0.9, 0.99));
			}
			if (Debug.KeyState(KeyCode.KC_U))
			{
				Debug.ClearKey(KeyCode.KC_U);
				Print(HealHitZones(10, false, true));	
			}
			if (Debug.KeyState(KeyCode.KC_I))
			{
				Debug.ClearKey(KeyCode.KC_I);
				
				SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, 10, hitPosDirNorm, GetGame().GetPlayerController().GetControlledEntity(), GetHitZoneByName("RThigh"), Instigator.CreateInstigator(GetGame().GetPlayerController().GetControlledEntity()), null, -1, -1);
				HandleDamage(damageContext);
			}
			if (Debug.KeyState(KeyCode.KC_O))
			{
				Debug.ClearKey(KeyCode.KC_O);
				
				SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, 10, hitPosDirNorm, GetGame().GetPlayerController().GetControlledEntity(), GetHitZoneByName("LThigh"), Instigator.CreateInstigator(GetGame().GetPlayerController().GetControlledEntity()), null, -1, -1);
				HandleDamage(damageContext);
			}
			if (Debug.KeyState(KeyCode.KC_P))
			{
				Debug.ClearKey(KeyCode.KC_P);
				AddParticularBleeding("RThigh");
			}
			if (Debug.KeyState(KeyCode.KC_T))
			{
				Debug.ClearKey(KeyCode.KC_T);
				AddParticularBleeding("LThigh");
			}
			if (Debug.KeyState(KeyCode.KC_K))
			{
				Debug.ClearKey(KeyCode.KC_K);
				FullHeal();
			}
		}
	}
#endif
};