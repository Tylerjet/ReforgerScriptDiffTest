[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting8 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FIREPOZ_2");
		m_fWaypointCompletionRadius = 2;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FireTwo", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};