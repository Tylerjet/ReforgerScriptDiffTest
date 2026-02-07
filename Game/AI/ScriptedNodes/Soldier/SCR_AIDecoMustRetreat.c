class SCR_AIDecoMustRetreat : DecoratorScripted
{
	SCR_AICombatComponent m_CombatComponent;
	
	//-------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComponent)
			return false;
		
		return m_CombatComponent.m_bMustRetreat;
	}
	
	//-------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true; }
};