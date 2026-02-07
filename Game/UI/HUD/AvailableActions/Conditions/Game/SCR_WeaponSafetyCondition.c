//------------------------------------------------------------------------------------------------
/*[BaseContainerProps()]
class SCR_WeaponFiremodeCondition : SCR_AvailableActionCondition
{	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWeaponFiremodeType))]
	protected EWeaponFiremodeType m_iValue;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (data.GetIsCharacterGettingIn())
			return false;
		
		if (data.GetIsCharacterGettingOut())
			return false;
		
		if (!data.GetCurrentWeapon())
			return false;
		
		bool result = (data.GetCurrentWeapon().GetCurrentFireModeType() == m_iValue);
		
		return GetReturnResult(result);
	}
}*/