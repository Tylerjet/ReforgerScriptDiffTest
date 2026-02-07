[EntityEditorProps(insertable: false)]
class SCR_TutorialBallistic2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

class SCR_TutorialBallistic2: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		m_bConditionPassCheck = true;
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
   		PlaySoundSystem("Balistic", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};