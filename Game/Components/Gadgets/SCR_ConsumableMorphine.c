//! Morphine effect
[BaseContainerProps()]
class SCR_ConsumableMorphine : SCR_ConsumableEffectHealthItems
{
	[Attribute("10", UIWidgets.EditBox, "Regeneration speed of related hitzone when consuming this item", category: "Regeneration")]
	protected float m_fItemRegenerationSpeed;
	
	[Attribute("10", UIWidgets.EditBox, "Regeneration duration of related hitzone when consuming this item in seconds", category: "Regeneration")]
	protected float m_fItemRegenerationDuration;	
	
	[Attribute("0", UIWidgets.EditBox, "Total amount of regeneration that will be applied to the related hitzone. Will be ignored if m_fItemRegenerationDuration > 0", category: "Regeneration")]
	protected float m_fItemAbsoluteRegenerationAmount;
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;
		
		array<HitZone> physicalHitZones = {};
		damageMgr.GetPhysicalHitZones(physicalHitZones);
		foreach (HitZone physicalHitZone: physicalHitZones)
		{
			ScriptedHitZone scriptedHitZone = ScriptedHitZone.Cast(physicalHitZone);
			if (!scriptedHitZone)
				continue;

			if (scriptedHitZone.GetDamageOverTime(EDamageType.REGENERATION))
				continue;
			
			scriptedHitZone.CustomRegeneration(target, m_fItemRegenerationDuration, m_fItemRegenerationSpeed, m_fItemAbsoluteRegenerationAmount);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		if (damageMgr.GetDefaultHitZone().GetHealthScaled() < 1)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group)
	{
		return super.CanApplyEffect(target, user);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableMorphine()
	{
		m_eConsumableType = EConsumableType.Morphine;
	}
}