class SCR_CampaignTutorialStage16Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage16 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_OBSTACLE_END");
		m_fWaypointCompletionRadius = 2;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_ObstacleCourseFinish" + CreateString("#AR-Keybind_SpeedSprint","CharacterSprint") + CreateString("#AR-Keybind_SpeedSprintToggle","CharacterSprintToggle"), duration: -1);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Sprint", 12, "", "", "", "");
	}
};