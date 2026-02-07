class SCR_AIIsFriendlyInAim : DecoratorScripted
{
	protected SCR_AICombatComponent m_CombatComponent;
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComponent)
			return false;
		
		return m_CombatComponent.IsFriendlyInAim();
	}
		
	override protected void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		
		if (!ent)
			NodeError(this, owner, "SCR_AIIsFriendlyInAim must be used with soldiers");
		
		m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));
		
		if (!m_CombatComponent)
			NodeError(this, owner, "Didn't find SCR_AICombatComponent");
	}
	
	protected override string GetOnHoverDescription()
	{
		return "SCR_AIIsFriendlyInAim: Returns true when we have a friendly unit in aim. Used for checking against friendly fire.";
	}
};
