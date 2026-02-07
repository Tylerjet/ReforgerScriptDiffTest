[EntityEditorProps(insertable: false)]
class SCR_TutorialBallistic1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

class SCR_TutorialBallistic1: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_BALLISTIC");
		m_fWaypointCompletionRadius = 2;

		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
    	PlaySoundSystem("MoveBalistic", true);
		HintOnVoiceOver();
	}
};