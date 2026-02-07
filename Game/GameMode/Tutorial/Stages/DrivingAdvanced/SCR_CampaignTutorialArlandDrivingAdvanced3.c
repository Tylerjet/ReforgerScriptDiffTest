[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced3 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_2");
		m_fWaypointCompletionRadius = 10;
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("DrivingWet", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "DrivingWet", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle() && !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};