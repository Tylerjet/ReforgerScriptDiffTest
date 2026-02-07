class SCR_CampaignTutorialStage20Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage20 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FIREPOZ_1");
		m_fWaypointCompletionRadius = 2;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringPosition1", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};