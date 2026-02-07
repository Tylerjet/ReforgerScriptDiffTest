[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDriving12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDriving12 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_39");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Square", true);
		m_TutorialComponent.SetWaypointMiscImage("SERPENTINE", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};