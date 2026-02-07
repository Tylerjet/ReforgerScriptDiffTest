class SCR_InspectionUserAction : SCR_InventoryAction
{
	protected BaseWeaponComponent m_WeaponComponent;

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_WeaponComponent)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_WeaponComponent = WeaponComponent.Cast(pOwnerEntity.FindComponent(WeaponComponent));
	}
};