class SCR_HitZone : HitZone
{
	protected ref ScriptInvokerVoid m_OnHealthChanged;
	protected ref ScriptInvoker m_OnDamageStateChanged; // TODO: make it a ScriptInvoker w/ ScriptedHitZone
	
	//------------------------------------------------------------------------------------------------
	//! Hit zone group getter to be overridden
	//! \return Hit zone group
	EHitZoneGroup GetHitZoneGroup()
	{
		return EHitZoneGroup.VIRTUAL;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when hitzone damage changes.
	//! \param createNew only create a script invoker if this is set to true
	//! \return Script invoker
	ScriptInvokerVoid GetOnHealthChanged(bool createNew = true)
	{
		if (!m_OnHealthChanged && createNew)
			m_OnHealthChanged = new ScriptInvokerVoid();

		return m_OnHealthChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when hitzone damage state changes.
	//! \return Script invoker
	ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!m_OnDamageStateChanged && createNew)
			m_OnDamageStateChanged = new ScriptInvoker();

		return m_OnDamageStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Owner entity of the HitZone
	IEntity GetOwner()
	{
		HitZoneContainerComponent container = GetHitZoneContainer();
		if (container)
			return container.GetOwner();
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHealthSet()
	{
		if (m_OnHealthChanged)
			m_OnHealthChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the damage state changes.
	override protected void OnDamageStateChanged()
	{
		if (m_OnDamageStateChanged)
			m_OnDamageStateChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (damageContext.damageValue == 0)
			return 0;

		if (damageContext.damageType == EDamageType.TRUE)
			return damageContext.damageValue;

		//apply base multiplier
		float effectiveDamage = damageContext.damageValue * GetBaseDamageMultiplier();
		//apply damage multiplier for this specific damage type
		effectiveDamage *= GetDamageMultiplier(damageContext.damageType);
		
		//DOT doesn't get affected by damage reduction/thresholds, and neither does healing.
		if (isDOT || effectiveDamage < 0)
			return effectiveDamage;

		//apply flat damage reduction
		effectiveDamage -= GetDamageReduction();

		//if its less than the damage threshold we discard the damage.
		//if the damage to take becomes negative (healing) because of the flat damage reduction, this should reset it back to 0 dmg to take.
		if (effectiveDamage < GetDamageThreshold())
			effectiveDamage = 0;

		return effectiveDamage;
	}
	
	//	TODO@FAC. To be implemented in gamecode in future also add regen scale multiplier!!!
	//------------------------------------------------------------------------------------------------
	//! Give any particular hit zone a regeneration over time
	//! \param target
	//! \param itemRegenerationDuration
	//! \param itemAbsoluteRegenerationAmount
	void CustomRegeneration(IEntity target, float itemRegenerationDuration, float itemRegenerationSpeed = 0, float itemAbsoluteRegenerationAmount = 0)
	{
		if (itemRegenerationSpeed)	// If a regeneration time is set, regen will occur for given amount of time at the itemRegenerationSpeed
		{
			GetGame().GetCallqueue().CallLater(RemoveCustomRegeneration, itemRegenerationDuration * 1000, false, target, itemRegenerationSpeed);
		}
		else if (itemAbsoluteRegenerationAmount)	// If an absolute regen amount is set instead of a duration, the regen will last until the amount of points has been distributed at the itemRegenerationSpeed
		{
			itemRegenerationSpeed = itemAbsoluteRegenerationAmount / itemRegenerationDuration;
			GetGame().GetCallqueue().CallLater(RemoveCustomRegeneration, itemRegenerationDuration * 1000, false, target, itemRegenerationSpeed);
		}
		else	// If regenerating value is 0, quit.
		{
			Print("Consumable with regenerating abilities was used but no duration or amount was defined", LogLevel.WARNING);
			return;
		}
		
		float currentRegen = GetDamageOverTime(EDamageType.HEALING);
		SetDamageOverTime(EDamageType.HEALING, currentRegen + itemRegenerationSpeed * -1);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//!
	//! \param target
	//! \param itemRegenerationSpeed
	void RemoveCustomRegeneration(IEntity target, float itemRegenerationSpeed)
	{
		// if healing was removed somehow before the regeneration was finished, don't remove again
		if (GetDamageOverTime(EDamageType.HEALING) + itemRegenerationSpeed > 0)
			SetDamageOverTime(EDamageType.HEALING, 0);
		else
			SetDamageOverTime(EDamageType.HEALING, (GetDamageOverTime(EDamageType.HEALING) + itemRegenerationSpeed));
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void DrawDebug();
#endif
}
