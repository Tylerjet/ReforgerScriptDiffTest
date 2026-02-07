//------------------------------------------------------------------------------------------------
//! Saline bag effect
[BaseContainerProps()]
class SCR_ConsumableSalineBag : SCR_ConsumableEffectHealthItems
{
	[Attribute("10", UIWidgets.EditBox, "Regeneration duration of related hitzone when consuming this item in seconds", category: "Consumable")]
	protected float m_fItemRegenerationDuration;	
	
	[Attribute("0", UIWidgets.EditBox, "Total amount of regeneration that will be applied to the related hitzone. Will be ignored if m_fItemRegenerationDuration > 0", category: "Consumable")]
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
		
		SCR_CharacterBloodHitZone bloodHitZone = SCR_CharacterBloodHitZone.Cast(damageMgr.GetBloodHitZone());
		if (!bloodHitZone)
			return;
		
		bloodHitZone.CustomRegeneration(target, m_fItemRegenerationDuration, 0, m_fItemAbsoluteRegenerationAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (damageMgr)
			return (damageMgr.GetBloodHitZone().GetDamageState() != EDamageState.UNDAMAGED);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group)
	{
		if (group == ECharacterHitZoneGroup.LEFTARM || group == ECharacterHitZoneGroup.RIGHTARM)
			return super.CanApplyEffect(target, user);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableSalineBag()
	{
		m_eConsumableType = EConsumableType.Saline;
	}
	
};