[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAction
{
	SCR_AIGroup m_AIGroup;
	IEntity m_IEntity;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		if (!targetAIGroup)
			return;
		
		m_AIGroup = targetAIGroup;
		m_IEntity = entity;

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate();
}