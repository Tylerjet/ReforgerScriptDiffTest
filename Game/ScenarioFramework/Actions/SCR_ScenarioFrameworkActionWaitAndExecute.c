[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionWaitAndExecute : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "How long to wait before executing action")]
	int							m_iDelayInSeconds;

	[Attribute(desc: "If this is set to a number larger than  Delay In Seconds, it will randomize resulted delay between these two values")]
	int m_iDelayInSecondsMax;

	[Attribute(UIWidgets.CheckBox, desc: "If true, it will activate actions in looped manner using Delay settings as the frequency. If randomized, it will randomize the time each time it loops.")]
	bool m_bLooped;

	[Attribute(defvalue: "1", desc: "Which actions will be executed once set time passes", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	protected int m_iDelay;
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	void ExecuteActions(IEntity object)
	{
		if (m_bLooped)
		{
			m_iDelay = m_iDelayInSeconds;
			Math.Randomize(-1);
			if (m_iDelayInSecondsMax > m_iDelayInSeconds)
				m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);
		}

		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(object);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		m_iDelay = m_iDelayInSeconds;
		Math.Randomize(-1);
		if (m_iDelayInSecondsMax > m_iDelayInSeconds)
			m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);

		//Used to delay the call as it is the feature of this action
		SCR_ScenarioFrameworkSystem.GetCallQueue().CallLater(ExecuteActions, m_iDelay * 1000, m_bLooped, object);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}