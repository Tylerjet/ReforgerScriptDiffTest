[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture7 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("TownBaseBeauregard");
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Conflict_CaptureArizona", true);
		HintOnVoiceOver();

		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
	}
};