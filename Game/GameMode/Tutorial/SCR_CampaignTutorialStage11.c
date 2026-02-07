class SCR_CampaignTutorialStage11Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage11 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WALL2");
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.3;
		GetGame().GetCallqueue().Remove(m_TutorialComponent.Check3rdPersonViewUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};