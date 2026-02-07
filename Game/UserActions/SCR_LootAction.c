class SCR_LootAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	CharacterControllerComponent m_pCharacterControllerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		auto vicinity = CharacterVicinityComponent.Cast( pUserEntity.FindComponent( CharacterVicinityComponent ));
		if ( !vicinity )
			return;
		
		vicinity.SetItemOfInterest(pOwnerEntity);
		manager.SetLootStorage( pOwnerEntity );
 	 	manager.OpenInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (m_pCharacterControllerComponent && !m_pCharacterControllerComponent.IsDead())
			return false;
		
		return super.CanBeShownScript(user);
	}
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_pCharacterControllerComponent = CharacterControllerComponent.Cast(
			pOwnerEntity.FindComponent(CharacterControllerComponent)
		);
	}
	#endif
};