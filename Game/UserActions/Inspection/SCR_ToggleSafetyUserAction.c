class SCR_ToggleSafetyUserAction : SCR_InspectionUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		charComp.SetSafety(!IsSafetyOn(), false);
	}

	bool IsSafetyOn()
	{
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();
		return (muzzle.GetCurrentFireMode().GetFiremodeType() == EWeaponFiremodeType.Safety);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (IsSafetyOn())
		{
			outName = WidgetManager.Translate("#AR-Keybind_WeaponSafety (%1)", "#AR-UserAction_State_On-UC");
		}
		else
		{
			outName = WidgetManager.Translate("#AR-Keybind_WeaponSafety (%1)", "#AR-UserAction_State_Off-UC");
		}
		return true;
	}
};