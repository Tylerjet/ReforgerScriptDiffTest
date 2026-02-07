[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture22Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture22 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 15;
		m_bConditionPassCheck = true;
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("Conflict_CaptureEnd");
		
		m_TutorialComponent.SetStagesComplete(9, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};