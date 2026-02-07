[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionDisableLOD : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "If set, AI will not change it's LOD.")]
	bool m_bDisableLOD;
	
	override void OnActivate()
	{
		if (!m_bDisableLOD)
			return;
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			agent.SetPermanentLOD(0);
		}
	}
}