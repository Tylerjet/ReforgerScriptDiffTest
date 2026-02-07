class SCR_LootAction : SCR_OpenStorageAction
{
	#ifndef DISABLE_INVENTORY
	CharacterControllerComponent m_pCharacterControllerComponent;
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