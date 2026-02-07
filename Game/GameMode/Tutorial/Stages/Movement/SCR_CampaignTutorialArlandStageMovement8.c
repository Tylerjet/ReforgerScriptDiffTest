[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement8 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WINDOW");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.6;
		PlaySoundSystem("Window", true);
		HintOnVoiceOver();
		m_TutorialComponent.SetWaypointMiscImage("JUMP", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};