class SCR_CampaignTutorialStage42Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage42 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_ANTENNA");
		m_fWaypointCompletionRadius = 5;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Barracks", "", -1);
	}
};