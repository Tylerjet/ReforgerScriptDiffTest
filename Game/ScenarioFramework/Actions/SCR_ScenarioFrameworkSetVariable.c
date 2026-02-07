[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkSetVariable : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;

	[Attribute(desc: "Value of the variable")]
	string m_sVariableValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;	
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;
		
		manager.SetVariableValue(m_sVariableName, m_sVariableValue);
	}
}