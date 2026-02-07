[BaseContainerProps()]
class SCR_CustomTriggerConditionsGeneric : SCR_CustomTriggerConditions
{
	[Attribute(desc: "Conditions that will be checked", category: "Activation")]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aGenericConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used.", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Trigger")]
	SCR_EScenarioFrameworkLogicOperators m_eConditionLogic;

	//------------------------------------------------------------------------------------------------
	//! Checks if all or any condition is met for trigger, sets status accordingly.
	//! \param[in] trigger Checks if all or any condition is met for trigger activation.
	override void CustomTriggerConditions(SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		bool conditionStatus = SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eConditionLogic, m_aGenericConditions, trigger);
		trigger.SetTriggerConditionsStatus(conditionStatus);
	}
}