[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 5;
		m_bConditionPassCheck = true;
		
		if (!m_TutorialComponent)
			return;
		
		m_TutorialComponent.StageReset_ResetSeizing();
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "Conflict_Start", false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};