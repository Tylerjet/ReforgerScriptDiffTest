[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting4 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FIREPOZ_1");
		m_fWaypointCompletionRadius = 2;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FireOne", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};