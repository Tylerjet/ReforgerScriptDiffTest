class SCR_FireModeSwitchUserAction : SCR_InspectionUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();
		return muzzle && muzzle.GetFireModesCount() > 2;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_WeaponComponent)
			return;
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();
		if (!muzzle)
			return;
		
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		charComp.SetFireMode(muzzle.GetNextFireModeIndex());
	}
};