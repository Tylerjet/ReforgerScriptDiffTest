[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced7 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_6");
		m_fWaypointCompletionRadius = 10;
		SCR_HintManagerComponent.HideHint();
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("Brakelock");
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "Brakelock", false);
	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
		{
			return true;
		}
		return false;
	}
};