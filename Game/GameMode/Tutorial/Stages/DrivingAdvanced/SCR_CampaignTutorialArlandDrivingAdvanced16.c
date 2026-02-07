[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced16 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_14");
		m_fWaypointCompletionRadius = 10;
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("SlowClimb");
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "SlowClimb", false);
	}
};