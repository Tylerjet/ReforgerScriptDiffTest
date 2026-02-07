[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage6Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage6 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		m_fWaypointCompletionRadius = 100;
		RegisterWaypoint("WP_HELICOURSE_FLIGHT1");
		PlaySoundSystem("Heli_Controls");
		HintOnVoiceOver();
	}
};