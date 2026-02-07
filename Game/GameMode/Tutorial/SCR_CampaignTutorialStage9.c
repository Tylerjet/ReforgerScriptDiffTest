class SCR_CampaignTutorialStage9Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage9 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FLAG");
		m_fWaypointCompletionRadius = 2;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Pole", duration: -1);
	}
};