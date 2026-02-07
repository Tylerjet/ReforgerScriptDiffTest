[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskLayer : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;

		return new SCR_ScenarioFrameworkParam<SCR_ScenarioFrameworkLayerTask>(SCR_ScenarioFrameworkLayerTask.Cast(gameModeManager.GetLastFinishedTaskLayer()));
	}
}