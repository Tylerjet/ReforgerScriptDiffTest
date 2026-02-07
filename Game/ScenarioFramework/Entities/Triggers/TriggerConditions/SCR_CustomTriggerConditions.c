//Baseclass that is supposed to be extended and filled with custom conditions
[BaseContainerProps()]
class SCR_CustomTriggerConditions
{
	//------------------------------------------------------------------------------------------------
	//! Used to init the class data according to the needs of the condition
	//! \param[in] trigger
	void Init(SCR_ScenarioFrameworkTriggerEntity trigger);

	//------------------------------------------------------------------------------------------------
	//! Performs specified logic checks when called
	//! \param[in] trigger
	void CustomTriggerConditions(SCR_ScenarioFrameworkTriggerEntity trigger);
}