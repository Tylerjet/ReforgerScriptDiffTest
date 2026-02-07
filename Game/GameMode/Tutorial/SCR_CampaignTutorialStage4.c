class SCR_CampaignTutorialStage4Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage4 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 0.5;
		m_fConditionCheckPeriod = 0.1;
		RegisterWaypoint("WP_OVERPASS");
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_Scaffolding", duration: -1);
	}
};