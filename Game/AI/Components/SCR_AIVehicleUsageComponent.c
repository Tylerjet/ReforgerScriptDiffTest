class SCR_AIVehicleUsageComponentClass : ScriptComponentClass
{
}

void SCR_AIOnVehicleDeleted(SCR_AIVehicleUsageComponent comp);
typedef func SCR_AIOnVehicleDeleted;

void SCR_AIOnVehicleDamageStateChanged(SCR_AIVehicleUsageComponent comp, EDamageState state);
typedef func SCR_AIOnVehicleDamageStateChanged;

//! This component should be attached to root entity of all vehicles and static turrets which are usable by AI.
//! In case of a static tripod inside composition, this component should be on static tripod though, not on composition root.
//! Other AI systems depend on this, so some AI functionality might not work with vehicles without this component.
class SCR_AIVehicleUsageComponent : ScriptComponent
{
	protected ref ScriptInvokerBase<SCR_AIOnVehicleDeleted> m_OnDeleted;
	protected ref ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged> m_OnDamageStateChanged;
	
	//------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnVehicleDeleted> GetOnDeleted()
	{
		if (!m_OnDeleted)
			m_OnDeleted = new ScriptInvokerBase<SCR_AIOnVehicleDeleted>();
		
		return m_OnDeleted;
	}
	
	//------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged> GetOnDamageStateChanged()
	{
		if (!m_OnDamageStateChanged)
			m_OnDamageStateChanged = new ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged>();
		
		return m_OnDamageStateChanged;
	}

	//---------------------------------------------------------------------------------------------------
	//! Finds the component on nearest parent of that entity, including that entity.
	//! This should help with vehicles which have benches as slots, or static weapons which are children of compositions.
	static SCR_AIVehicleUsageComponent FindOnNearestParent(notnull IEntity ent, out IEntity componentOwner)
	{
		SCR_AIVehicleUsageComponent comp = null;
		
		while (!comp && ent)
		{
			comp = SCR_AIVehicleUsageComponent.Cast(ent.FindComponent(SCR_AIVehicleUsageComponent));
			componentOwner = ent;
			ent = ent.GetParent();
		}
		
		return comp;
	}
		
	//---------------------------------------------------------------------------------------------------
	// PROTECTED / INTERNAL
	
	//------------------------------------------------------------------------------
	protected void OnDamageStateChanged(EDamageState damageState)
	{
		if (!m_OnDamageStateChanged)
			return;
		
		m_OnDamageStateChanged.Invoke(this, damageState);
	}
	
	//------------------------------------------------------------------------------
	protected void ~SCR_AIVehicleUsageComponent()
	{		
		if (m_OnDeleted)
			m_OnDeleted.Invoke(this);
	}
	
	//------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_DamageManagerComponent damageMgr = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
		if (damageMgr)
			damageMgr.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------
	static void ErrorNoComponent(notnull IEntity entity)
	{
		Print(string.Format("SCR_AIVehicleUsageComponent not found on entity: %1", entity), LogLevel.ERROR);
	}
}