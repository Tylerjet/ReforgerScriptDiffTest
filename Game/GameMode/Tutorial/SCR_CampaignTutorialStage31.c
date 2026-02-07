class SCR_CampaignTutorialStage31Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage31 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_2");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.HideHint, 10000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_MoveInJeep(m_WaypointEntity);
	}
};