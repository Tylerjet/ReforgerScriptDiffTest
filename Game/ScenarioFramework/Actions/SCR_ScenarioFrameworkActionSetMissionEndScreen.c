[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetMissionEndScreen : SCR_ScenarioFrameworkActionBase
{
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))]
	EGameOverTypes			m_eGameOverType;

	[Attribute()]
	LocalizedString m_sSubtitle;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(gamemode.FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		manager.SetMissionEndScreen(m_eGameOverType);

		SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverScreenMgr)
			return;

		SCR_GameOverScreenConfig m_GameOverScreenConfig = gameOverScreenMgr.GetGameOverConfig();
		if (!m_GameOverScreenConfig)
			return;

		SCR_BaseGameOverScreenInfo targetScreenInfo;
		m_GameOverScreenConfig.GetGameOverScreenInfo(m_eGameOverType, targetScreenInfo);
		if (!targetScreenInfo)
			return;

		SCR_BaseGameOverScreenInfoOptional optionalParams = targetScreenInfo.GetOptionalParams();
		if (!optionalParams)
			return;

		optionalParams.m_sSubtitle = m_sSubtitle;
	}
}