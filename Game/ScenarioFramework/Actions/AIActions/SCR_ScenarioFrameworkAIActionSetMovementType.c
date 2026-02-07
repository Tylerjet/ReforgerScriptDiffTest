[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetMovementType : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EMovementType.WALK.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(EMovementType), category: "Common")]
	EMovementType m_eAIMovementType;
	
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

			SCR_AIInfoComponent infoComponent = SCR_AIInfoComponent.Cast(agentEntity.FindComponent(SCR_AIInfoComponent));
			if (infoComponent)
				infoComponent.SetMovementType(m_eAIMovementType);
		}
	}
}