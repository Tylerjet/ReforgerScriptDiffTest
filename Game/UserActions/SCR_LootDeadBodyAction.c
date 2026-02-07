class SCR_LootDeadBodyAction : SCR_LootAction
{
	private DamageManagerComponent m_pDamageComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_pDamageComponent && m_pDamageComponent.GetState() != EDamageState.DESTROYED)
			return false;
		
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_pDamageComponent = DamageManagerComponent.Cast(pOwnerEntity.FindComponent(DamageManagerComponent));
	}
};