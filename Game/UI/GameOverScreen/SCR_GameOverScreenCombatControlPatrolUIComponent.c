class SCR_GameOverScreenCombatControlPatrolUIComponent: SCR_GameOverScreenContentUIComponent
{
	override void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.InitContent(endScreenUIContent);
		
		TextWidget debriefingWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sDebriefingName));
		
		debriefingWidget.SetText("Custom debrief");
	}
};