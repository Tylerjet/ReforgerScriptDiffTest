class SCR_FireModeSwitchUserAction : SCR_InspectionUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		return (super.CanBeShownScript(user) && m_WeaponComponent.GetCurrentMuzzle().GetFireModesCount() > 2);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();

		charComp.SetFireMode(muzzle.GetNextFireModeIndex());
	}
};