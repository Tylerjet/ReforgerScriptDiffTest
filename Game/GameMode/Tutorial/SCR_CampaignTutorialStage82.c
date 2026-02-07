class SCR_CampaignTutorialStage82Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage82 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_COMPAS_MOVE");
		m_fWaypointCompletionRadius = 40;
		m_fConditionCheckPeriod = 1;
		m_bShowWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Farmhouse", duration: -1);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 21000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_NoWaypoints", 15, "", "", "", "");
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};