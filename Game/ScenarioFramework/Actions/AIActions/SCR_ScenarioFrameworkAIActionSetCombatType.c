[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetCombatType : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAICombatType.NORMAL.ToString(), UIWidgets.ComboBox, "AI combat type", "", ParamEnumArray.FromEnum(EAICombatType), category: "Common")]
	EAICombatType m_eAICombatType;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetCombatType(m_eAICombatType);
		}
	}
}