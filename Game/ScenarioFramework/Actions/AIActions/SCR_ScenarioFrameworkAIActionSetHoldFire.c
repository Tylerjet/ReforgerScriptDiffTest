[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetHoldFire : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "If AI in the group should hold fire")]
	bool m_bHoldFire;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetHoldFire(m_bHoldFire);
		}
	}
}