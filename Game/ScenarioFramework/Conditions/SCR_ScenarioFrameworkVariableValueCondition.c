[BaseContainerProps()]
class SCR_ScenarioFrameworkVariableValueCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;

	[Attribute(desc: "Check if the variable has set this value or not.")]
	string m_sVariableValueToCheck;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return false;
		
		string outValue;
		manager.GetVariable(m_sVariableName, outValue);
		
		if (outValue.IsEmpty())
			return false;
		
		return outValue == m_sVariableValueToCheck;
	}
}