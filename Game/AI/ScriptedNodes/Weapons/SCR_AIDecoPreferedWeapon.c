class SCR_AIDecoPreferedWeapon : DecoratorScripted
{
	SCR_AICombatComponent m_CombatComponent;
	
	[Attribute(EWeaponType.WT_NONE.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EWeaponType))]
	EWeaponType m_eWeaponType;
	
	//-------------------------------------------------------------------------------------------
	static protected override string GetOnHoverDescription()
	{
		return "Returns true when value returned by SCR_AICombatComponent.GetPreferedWeapon() equals m_eWeaponType.";
	}

	//-------------------------------------------------------------------------------------------	
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComponent)
			return false;
		
		return m_CombatComponent.GetPreferedWeapon() == m_eWeaponType;
	}
	
	//-------------------------------------------------------------------------------------------
	override protected void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	override protected string GetNodeMiddleText()
	{
		return string.Format("Weapon Type: %1", typename.EnumToString(EWeaponType, m_eWeaponType));
	}
};