[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced20Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced20 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_18");
		m_fWaypointCompletionRadius = 10;
		m_TutorialComponent.SetWaypointMiscImage("LEFT", true);
		if (!m_TutorialComponent.GetVoiceSystem().IsPlaying())
			PlaySoundSystem("ParkTruck", true);
		else
			GetGame().GetCallqueue().CallLater(PlaySoundSystem, 1000, false, "ParkTruck", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};