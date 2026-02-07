[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced13 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("Truck");
		m_fWaypointCompletionRadius = 10;
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("GetInTruck", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "GetInTruck", true);
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