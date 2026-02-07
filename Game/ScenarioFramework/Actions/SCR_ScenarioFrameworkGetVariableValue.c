[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetVariableValue : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;
		
		string value;
		
		manager.GetVariable(m_sVariableName, value);
	}
}