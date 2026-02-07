class SCR_DamageManagerComponentClass: DamageManagerComponentClass
{

};

class SCR_DamageManagerComponent: DamageManagerComponent
{
	private int m_iInstigatorID;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Set instigator and his player ID using entity
	//! Checks for turret gunner if instigator is not a character, until drivers and gunners are properly tracked in gamecode
	void SetInstigatorEntity(IEntity instigator)
	{
		// Find turret gunners to give them credit for kills
		// Please remove this block when gamecode instigator detection properly accounts for turrets and drivers
		if (instigator && !ChimeraCharacter.Cast(instigator))
		{
			array<IEntity> occupants = {};
			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(instigator.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (compartmentManager)
				compartmentManager.GetOccupantsOfType(occupants, ECompartmentType.Turret);
			
			if (!occupants.IsEmpty())
				instigator = occupants[0];
		}
		
		m_iInstigatorID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(instigator);
		
		SetInstigator(instigator);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Read instigator from player ID if set previously
	IEntity GetInstigatorEntity()
	{
		IEntity instigator;
		if (m_iInstigatorID != 0)
			instigator = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iInstigatorID);
		
		if (!instigator)
			instigator = GetInstigator();
		
		return instigator;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Set instigator and his player ID using player ID
	void SetInstigatorID(int instigatorID)
	{
		m_iInstigatorID = instigatorID;
		
		IEntity instigator;
		if (instigatorID != 0)
			instigator = GetGame().GetPlayerManager().GetPlayerControlledEntity(instigatorID);
		
		SetInstigator(instigator);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Read instigator player ID if set previously
	int GetInstigatorID()
	{
		return m_iInstigatorID;
	}
	
	//------------------------------------------------------------------------------------------------
	/*! Neutralize the entity with a specific damage type, registering the killer entity.
	\param instigator Source of the damage
	*/
	void Kill(IEntity instigator = null)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		if (!instigator)
			instigator = owner;
		
		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;
		
		vector hitPosDirNorm[3];
		HandleDamage(EDamageType.TRUE, hitZone.GetMaxHealth(), hitPosDirNorm, owner, hitZone, instigator, null, -1, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Damage random physical hitzones up to damage amount
	\param damage Total damage to be applied
	\param type Type of damage
	\param instigator Entity that fired the projectile or otherwise caused the damage
	\param onlyPhysical Whether only physical hitzones should be damaged
	\param outMat [hitPosition, hitDirection, hitNormal]
	\param damageDefault Whether to damage default hitzone as well
	*/
	void DamageRandomHitZones(float damage, EDamageType type, IEntity instigator = null, bool onlyPhysical = true, vector outMat[3] = {}, bool damageDefault = true)
	{
		array<HitZone> hitZones = {};
		
		if (onlyPhysical)
			GetPhysicalHitZones(hitZones);
		else
			GetAllHitZones(hitZones);
		
		if (!damageDefault)
			hitZones.RemoveItem(GetDefaultHitZone());
		
		if (!instigator)
			instigator = GetOwner();
		
		while (damage > 0 && !hitZones.IsEmpty())
		{
			HitZone hitZone = hitZones.GetRandomElement();
			if (!hitZone)
			{
				hitZones.RemoveItem(hitZone);
				continue;
			}
			
			float hitZoneDamage = Math.Min(damage, hitZone.GetHealth() + hitZone.GetDamageReduction());
			damage -= hitZoneDamage;
			
			if (hitZoneDamage <= 0)
				continue;
			
			HandleDamage(type, hitZoneDamage, outMat, GetOwner(), hitZone, instigator, null, -1, -1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	bool CanBeHealed()
	{
		// Any damage to default hitzone
		if (GetDefaultHitZone().GetHealthScaled() < 1)
			return true;
		
		// Check if has damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		
		foreach (EDamageType type: damageTypes)
		{
			if (IsDamagedOverTime(type))
				return true;
		}
		
		// Check if any hitzone is damaged
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		foreach (HitZone hitZone: hitZones)
		{
			if (hitZone && hitZone.GetHealthScaled() < 1)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	void FullHeal()
	{
		// Heal damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		
		foreach (EDamageType type: damageTypes)
		{
			if (IsDamagedOverTime(type))
				RemoveDamageOverTime(type);
		}
		
		// Fix hitzones
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		foreach (HitZone hitZone: hitZones)
		{
			if (hitZone && hitZone.GetHealthScaled() < 1)
				hitZone.SetHealthScaled(1);
		}
	}
};
