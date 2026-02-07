[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture20Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture20 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Conflict_MHQInfo");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};