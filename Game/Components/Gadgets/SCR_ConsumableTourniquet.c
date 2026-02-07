//------------------------------------------------------------------------------------------------
//! Tourniquet effect
[BaseContainerProps()]
class SCR_ConsumableTourniquet: SCR_ConsumableEffectHealthItems
{
	//Cached to remember which bodypart to remove tourniquet from
	ECharacterHitZoneGroup m_eTargetHZGroup;
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{
		super.ApplyEffect(target, user, item, animParams);
		
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;
		
		m_eTargetHZGroup = damageMgr.FindAssociatedHitZoneGroup(animParams.m_intParam);

		SCR_TourniquetStorageComponent tqStorageComp = SCR_TourniquetStorageComponent.Cast(user.FindComponent(SCR_TourniquetStorageComponent));
		if (!tqStorageComp)
		{
			Print("SCR_TourniquetStorageComponent missing", LogLevel.ERROR);
			return;
		}
		
		tqStorageComp.AddTourniquetToSlot(target, m_eTargetHZGroup, item);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr || !damageMgr.IsDamagedOverTime(EDamageType.BLEEDING))
			return false;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMgr.GetAllExtremities(limbs);
		
		return damageMgr.GetMostDOTHitZone(EDamageType.BLEEDING, false, limbs);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMgr.GetAllExtremities(limbs);
		
		if (!limbs.Contains(group))
			return false;
		
		if (damageMgr.GetGroupDamageOverTime(group, EDamageType.BLEEDING) > 0)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ConsumableEffectAnimationParameters GetAnimationParameters(IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return null;

		EBandagingAnimationBodyParts bodyPartToBandage;
		
		if (group != ECharacterHitZoneGroup.VIRTUAL)
		{
			bodyPartToBandage = damageMgr.FindAssociatedBandagingBodyPart(group);
		}
		else
		{
			array<ECharacterHitZoneGroup> limbs = {};
			damageMgr.GetAllExtremities(limbs);
			
			HitZone hitzone = damageMgr.GetMostDOTHitZone(EDamageType.BLEEDING, false, limbs);
			if (!hitzone)
				return null;
			
			SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(hitzone);
			if (!charHitZone)
				return null;
			
			bodyPartToBandage = charHitZone.GetBodyPartToHeal();
		}
		
		if (bodyPartToBandage == EBandagingAnimationBodyParts.Invalid)
				return null;
		
		return new SCR_ConsumableEffectAnimationParameters(GetApplyToSelfAnimCmnd(target), 1, 0, 4.0, bodyPartToBandage, 0.0, false);	
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetTargetHitZoneGroup()
	{
		return m_eTargetHZGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableTourniquet()
	{
		m_eConsumableType = EConsumableType.Tourniquet;
	}
	
};