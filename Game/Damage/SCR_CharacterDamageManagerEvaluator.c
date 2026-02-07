class SCR_CharacterDamageManagerEvaluator : DamageEffectEvaluator
{
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BaseBulletDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		EffectCauseBleeding(effect, dmgManager);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_CollisionDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		EffectCauseBleeding(effect, dmgManager);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_FragmentationDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		EffectCauseBleeding(effect, dmgManager);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_MeleeDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		EffectCauseBleeding(effect, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BandageDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		array<HitZone> hitZonesOfGroup = {};
		damageMgr.GetHitZonesOfGroup(SCR_HitZone.Cast(effect.GetAffectedHitZone()).GetHitZoneGroup(), hitZonesOfGroup);

		array<ref PersistentDamageEffect> persistentEffects = damageMgr.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);

		foreach (PersistentDamageEffect persistentEffect : persistentEffects)
		{
			if (!SCR_BleedingDamageEffect.Cast(persistentEffect))
				continue;

			foreach (HitZone hz : hitZonesOfGroup)
			{
				if (persistentEffect.GetAffectedHitZone() == hz)
				{
					damageMgr.TerminateDamageEffect(persistentEffect);
					break;
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*!
	Asks the damagemanager to add a bleeding when conditions are met. Only instantDamageEffects can start bleedings
	\param effect - Damage effect causing the bleeding. Must have a valid total damage amount and an affectedHitZone
	\param dmgManager - ParentDamageManager which owns this evaluator
	*/
	private void EffectCauseBleeding(SCR_InstantDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
		if (!affectedHitZone)
			return;

		if (effect.GetTotalDamage() < affectedHitZone.GetCriticalDamageThreshold() * affectedHitZone.GetMaxHealth())
			return; 
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!damageMgr)
			return;
		
		damageMgr.AddBleedingEffectOnHitZone(affectedHitZone);
	}
}
