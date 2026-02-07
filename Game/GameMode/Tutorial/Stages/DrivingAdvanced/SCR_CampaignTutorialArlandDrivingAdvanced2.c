[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced2 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("Hummer");
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
		
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("GetInHmw", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "GetInHmw", true);
		
		m_fWaypointCompletionRadius = 10;
		
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
		{
			SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		}
		return m_Player.IsInVehicle() && !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};