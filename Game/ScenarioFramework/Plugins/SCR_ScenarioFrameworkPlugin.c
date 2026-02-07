[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPlugin : ScriptAndConfig
{
	protected SCR_ScenarioFrameworkLayerBase m_Object;

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkLayerBase GetObject()
	{
		return m_Object;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		m_Object = object;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)
	{
	}
}