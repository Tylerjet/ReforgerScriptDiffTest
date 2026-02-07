class SCR_CampaignTutorialStage43Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage43 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_HQ");
		m_fWaypointCompletionRadius = 5;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Antenna", "", -1);
	}
};