[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBasedOnConditions : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Action activation conditions")]
	protected ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aActivationConditions;
	
	[Attribute(desc: "Which actions will be executed once set time passes")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(object))
				return;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(object);
		}
	}
}