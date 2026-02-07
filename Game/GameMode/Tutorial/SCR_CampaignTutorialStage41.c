class SCR_CampaignTutorialStage41Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage41 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_TOUR_BARRACKS");
		m_fWaypointCompletionRadius = 5;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Armory", "", -1);
	}
};