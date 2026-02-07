class ScriptedHitZone : HitZone
{
	private ref ScriptInvoker Event_EOnHealthSet;
	private ref ScriptInvoker Event_EOnDamageStateChanged;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when hitzone damage changes.
	\return Script invoker
	*/
	ref notnull ScriptInvoker GetOnHealthChanged(bool createNew = true)
	{
		if (!Event_EOnHealthSet && createNew)
			Event_EOnHealthSet = new ScriptInvoker();
		return Event_EOnHealthSet;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when hitzone damage state changes.
	\return Script invoker
	*/
	ref notnull ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!Event_EOnDamageStateChanged && createNew)
			Event_EOnDamageStateChanged = new ScriptInvoker();
		return Event_EOnDamageStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Owner entity of the HitZone
	*/
	IEntity GetOwner()
	{
		HitZoneContainerComponent container = GetHitZoneContainer();
		if (container)
			return container.GetOwner();
		
		return null;
	}
	
	//! Called when hit zone is initialized
	void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent);
	
	//------------------------------------------------------------------------------------------------
	//! Called every frame if this hitzone is simulated (SetSimulationState)
	void OnSimulate(IEntity owner, float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone. 
	This method is not guaranteed to be in sync with server, use at your own risk.
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param damageSource Projectile object
	\param instigator Damage source parent entity (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	void OnLocalDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity damageSource, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
	
	//------------------------------------------------------------------------------------------------
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
	void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage is being set by means different than damage over time.
	*/
	protected void OnHealthSet()
	{
		if (Event_EOnHealthSet)
			Event_EOnHealthSet.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage state changes.
	*/
	protected void OnDamageStateChanged()
	{
		if (Event_EOnDamageStateChanged)
			Event_EOnDamageStateChanged.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the max damage changes.
	*/
	protected void OnMaxHealthChanged();
	
	/*!
	Calculates the amount of damage a hitzone will receive.
		
	\param EDamageType damageType, damage type
	\param float rawDamage, incoming damage, without any modifiers taken into account
	\param IEntity hitEntity, damaged entity
	\param HitZone struckHitZone, hitzone to damage
	\param IEntity damageSource, projectile
	\param IEntity damageSourceGunner, damage source instigator 
	\param IEntity damageSourceParent, damage source parent entity (soldier, vehicle)
	\param const GameMaterial hitMaterial, hit surface physics material
	\param int colliderID, collider ID - if it exists
	\param const inout vector hitTransform[3], hit position, direction and normal
	\param const vector impactVelocity, projectile velocity in time of impact
	\param int nodeID, bone index in mesh obj
	\param bool isDOT, true if this is a calculation for DamageOverTime 
	*/
	float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, IEntity damageSourceGunner, IEntity damageSourceParent, const GameMaterial hitMaterial, int colliderID, const inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		if (rawDamage == 0)
			return 0;

		if (damageType == EDamageType.TRUE)
			return rawDamage;

		//apply base multiplier
		float effectiveDamage = rawDamage * GetBaseDamageMultiplier();
		//apply damage multiplier for this specific damage type
		effectiveDamage *= GetDamageMultiplier(damageType);

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
	/*!
	Give any particular hitzone a regeneration over time
	*/
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
		
		float currentRegen = GetDamageOverTime(EDamageType.FIRE);
		SetDamageOverTime(EDamageType.FIRE, currentRegen + itemRegenerationSpeed * -1);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void RemoveCustomRegeneration(IEntity target, float itemRegenerationSpeed)
	{
		SetDamageOverTime(EDamageType.FIRE, (GetDamageOverTime(EDamageType.FIRE) + itemRegenerationSpeed) );
	}
};