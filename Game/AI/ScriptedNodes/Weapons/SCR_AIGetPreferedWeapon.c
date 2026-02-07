// Node return prefered weapon from combat component
class SCR_AIGetPreferedWeapon: AITaskScripted
{
	static const string PORT_WEAPON_TYPE = "WeaponType";
	
	SCR_AICombatComponent m_CombatComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int weaponType;
		
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_WEAPON_TYPE, m_CombatComponent.GetPreferedWeapon());
				

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_WEAPON_TYPE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};