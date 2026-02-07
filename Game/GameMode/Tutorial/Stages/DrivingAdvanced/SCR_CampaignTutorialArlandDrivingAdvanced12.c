[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced12 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_11");
		m_fWaypointCompletionRadius = 10;
		m_TutorialComponent.SetWaypointMiscImage("GETOUT", true);
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("Park", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "Park", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
		{
			SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		}
		return !m_Player.IsInVehicle() && !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}

};