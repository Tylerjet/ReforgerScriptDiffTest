
/*!
\addtogroup Components
\{
*/

class SCR_ExtendedDamageManagerComponentClass : ExtendedDamageManagerComponentClass
{
}

class SCR_ExtendedDamageManagerComponent : ExtendedDamageManagerComponent
{
	/*
	IMPORTANT: ===========================================================================================
	The following API will not be useful for damageManagers inheriting from ExtendedDamageManagerComponent
		//void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz);
		//void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz);
		//bool IsDamagedOverTime(EDamageType dType)
		//float GetDamageOverTime(EDamageType dType);
		//void RemoveDamageOverTime(EDamageType dType);
	
	DamageEffects logic replaces the conventional damageOverTime, and has it's own API.
	- Instead of OnDamageOverTimeAdded() use OnDamageEffectAdded()
	- Instead of OnDamageOverTimeRemoved() use OnDamageEffectRemoved()
	- Instead of IsDamagedOverTime() use 
		- SCR_CharacterDamageManagerComponent.IsBleeding()
		- GetPersistentEffects()
		- GetAllPersistentEffectsOnHitZone() 
		- GetAllPersistentEffectsOfType()
	- Instead of GetDamageOverTime() use
		- SCR_RegeneratingHitZone.GetHitZoneDamageOverTime()
		- SCR_CharacterBloodHitZone.GetTotalBleedingAmount()
	*/
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageEffectAdded()
	{
		return GetScriptedDamageManagerData().GetOnDamageEffectAdded();
	}	
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageEffectRemoved()
	{
		return GetScriptedDamageManagerData().GetOnDamageEffectRemoved();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectAdded(dmgEffect);
		
		if (s_aDamageManagerData.IsIndexValid(m_iDamageManagerDataIndex))
		{
			ScriptInvoker invoker = s_aDamageManagerData.Get(m_iDamageManagerDataIndex).GetOnDamageEffectAdded(false);
			if (invoker)
				invoker.Invoke(dmgEffect);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectAdded(dmgEffect);
		
		if (s_aDamageManagerData.IsIndexValid(m_iDamageManagerDataIndex))
		{
			ScriptInvoker invoker = s_aDamageManagerData.Get(m_iDamageManagerDataIndex).GetOnDamageEffectRemoved(false);
			if (invoker)
				invoker.Invoke(dmgEffect);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref PersistentDamageEffect> GetAllPersistentEffectsOnHitZone(HitZone hitzone)
	{
		array<ref PersistentDamageEffect> damageEffects = {};

		GetPersistentEffects(damageEffects);

		if (damageEffects.IsEmpty())
			return damageEffects;

		array<ref PersistentDamageEffect> allEffectsOnHitZone = {};

		foreach (ref PersistentDamageEffect effect : damageEffects)
		{
			if (effect.GetAffectedHitZone() == hitzone)
				allEffectsOnHitZone.Insert(effect);
		}

		return allEffectsOnHitZone;
	}

	//------------------------------------------------------------------------------------------------
	array<ref PersistentDamageEffect> FilterEffectsByHitZone(array<ref PersistentDamageEffect> damageEffects, HitZone hitzone)
	{
		array<ref PersistentDamageEffect> filteredDamageEffects = {};

		if (damageEffects.IsEmpty())
			return filteredDamageEffects;

		foreach (ref PersistentDamageEffect effect : damageEffects)
		{
			if (effect.GetAffectedHitZone() == hitzone)
				filteredDamageEffects.Insert(effect);
		}

		return filteredDamageEffects;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref PersistentDamageEffect> GetAllPersistentEffectsOfType(typename effectTypename)
	{
		array<ref PersistentDamageEffect> damageEffects = {};

		GetPersistentEffects(damageEffects);

		if (damageEffects.IsEmpty())
			return damageEffects;

		array<ref PersistentDamageEffect> allEffectsOfType = {};

		foreach (ref PersistentDamageEffect effect : damageEffects)
		{
			if (effect.ClassName() == effectTypename.ToString())
				allEffectsOfType.Insert(effect);
		}

		return allEffectsOfType;
	}

	//------------------------------------------------------------------------------------------------
	array<ref PersistentDamageEffect> FilterEffectsByType(array<ref PersistentDamageEffect> damageEffects, typename effectTypename)
	{
		array<ref PersistentDamageEffect> filteredDamageEffects = {};

		if (damageEffects.IsEmpty())
			return filteredDamageEffects;

		foreach (ref PersistentDamageEffect effect : damageEffects)
		{
			if (effect.ClassName() == effectTypename.ToString())
				filteredDamageEffects.Insert(effect);
		}

		return filteredDamageEffects;
	}
}
