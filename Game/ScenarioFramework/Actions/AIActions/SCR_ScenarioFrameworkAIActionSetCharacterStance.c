[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetCharacterStance : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: ECharacterStance.STAND.ToString(), UIWidgets.ComboBox, "AI character stance", "", ParamEnumArray.FromEnum(ECharacterStance), category: "Common")]
	ECharacterStance m_eAICharacterStance;
	
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

			SCR_AIInfoComponent infoComponent = SCR_AIInfoComponent.Cast(agentEntity.FindComponent(SCR_AIInfoComponent));
			if (infoComponent)
				infoComponent.SetStance(m_eAICharacterStance);
		}
	}
}