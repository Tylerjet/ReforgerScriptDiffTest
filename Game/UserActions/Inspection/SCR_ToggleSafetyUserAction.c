class SCR_ToggleSafetyUserAction : SCR_InspectionUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();
		bool safetyOn = (muzzle.GetCurrentFireMode().GetFiremodeType() == EWeaponFiremodeType.Safety);

		charComp.SetSafety(!safetyOn, false);
	}
};