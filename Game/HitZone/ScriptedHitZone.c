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
			Event_EOnDamageStateChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the max damage changes.
	*/
	protected void OnMaxHealthChanged();
};