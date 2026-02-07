class SCR_CampaignTutorialStage15Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage15 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_OFF");
		m_fWaypointCompletionRadius = 3;
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER) && vector.DistanceSq(m_Player.GetOrigin(), m_WaypointEntity.GetOrigin()) < 2;
	}
};